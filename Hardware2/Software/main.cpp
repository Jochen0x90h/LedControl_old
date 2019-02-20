#define LED_COUNT 300

#include <stddef.h>
#include <stdint.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include "util.h"
#include "generated/ledSerial.h"
#include "Bitmap.h"
#include "Pin.h"
#include "DmaChannel.h"
#include "Flash.h"

// fonts
#include "tahoma_8pt.h"


// STM32L433CC (48 pins, 256 KB flash)
//   Flash range: 0x0800 0000 - 0x0803 FFFF, page 0 - 127, each page 2 kb
// Data Sheet: https://www.st.com/resource/en/datasheet/stm32l433cc.pdf
//   Page 74: Alternative functions for io pins
// Reference Manual: https://www.st.com/content/ccc/resource/technical/document/reference_manual/group0/b0/ac/3e/8f/6d/21/47/af/DM00151940/files/DM00151940.pdf/jcr:content/translations/en.DM00151940.pdf
//   Page 77: Flash
//   Page 295: DMA
//   Page 716: Timer encoder interface mode


// clocks
void setupClocks() {
	// enbable HSI16 for SYSCLK and HSI48 for USB
	rcc_osc_on(RCC_HSI16);
	//rcc_osc_on(RCC_HSI48);
	
	// configure flash
	// 
	flash_prefetch_enable();
	flash_set_ws(4); // max CPU speed 0: 16MHz, 1: 32MHz, 2: 48MHz, 3: 64MHz, 4: 80MHz
	flash_dcache_enable();
	flash_icache_enable();

	// 16MHz / 1 * 8 / 2 = 64MHz main pll
	rcc_set_main_pll(
		RCC_PLLCFGR_PLLSRC_HSI16, // source is HSI16
		1, // / 1
		8, // * 8
		0,
		0,
		RCC_PLLCFGR_PLLR_DIV2); // / 4
	
	// enable PLL (can do other things before we wait for PLL to lock)
	rcc_osc_on(RCC_PLL);

	// enable clocks for the peripherals we use
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_DMA1);
	//rcc_periph_clock_enable(RCC_DMA2);
	rcc_periph_clock_enable(RCC_SYSCFG);

	// set frequency variables
	rcc_ahb_frequency = 64000000;
	rcc_apb1_frequency = 64000000;
	rcc_apb2_frequency = 64000000;

	// wait for PLL to lock
	rcc_wait_for_sysclk_status(RCC_PLL);

	// switch SYSCLK to PLL
	rcc_set_sysclk_source(RCC_CFGR_SW_PLL);
}

// debug LED
/*#define DEBUG_LED_PORT GPIOB
#define DEBUG_LED_PIN GPIO9
void setupDebugLed() {
	gpio_mode_setup(DEBUG_LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DEBUG_LED_PIN);
	gpio_set_output_options(DEBUG_LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, DEBUG_LED_PIN);
}*/


class Encoder {
public:
	template <uint32_t TIMER, uint32_t CH1_PIN, uint32_t CH2_PIN>
	void setup(uint32_t period = 0xffffffff) {
		this->timer = TIMER;
		rccClockEnable<TIMER>();

		// configure pins
		Pin().setup<CH1_PIN, TIMER, CH1>().setup(Pin::Resistor::PULLDOWN);
		Pin().setup<CH2_PIN, TIMER, CH2>().setup(Pin::Resistor::PULLDOWN);
		
		// configure timer
		timer_set_period(TIMER, period);
		timer_slave_set_mode(TIMER, TIM_SMCR_SMS_EM3); // encoder mode
		timer_ic_set_input(TIMER, TIM_IC1, TIM_IC_IN_TI1); // channel 1 on input 1
		timer_ic_set_input(TIMER, TIM_IC2, TIM_IC_IN_TI2); // channel 2 on input 2
	
		// clear
		timer_set_counter(TIMER, 0);

		// enable timer
		timer_enable_counter(TIMER);
	}

	uint32_t getValue() {
		return timer_get_counter(this->timer);
	}

	void setValue(uint32_t value) {
		timer_set_counter(this->timer, value);
	}

protected:
	uint32_t timer;
};


class Button {
public:
	void setup(uint32_t buttonPin) {
		this->pin.setup(pin).setup(Pin::Mode::INPUT).setup(Pin::Resistor::PULLDOWN);
		this->state = false;
	}
	
	// report press of button once until button is pressed again
	bool pressed() {
		bool nextState = this->pin;
		bool result = nextState && !this->state;
		this->state = nextState;
		return result;
	}
	
protected:
	Pin pin;
	bool state;
};

/**
 * Display init sequence
 * https://github.com/imxieyi/esp32-i2c-ssd1306-oled/blob/master/main/ssd1306.cpp
 */
uint8_t const displayInit[] = {
	0xae, // display off
	0xd5, 0x80, // clock divide ratio and oscillator frequency
	0xa8, 63, // multiplex ratio = 1/64
	0xc8, // output scan direction, vertical flip (c0/c8)
	0xd3, 0, // display offset = 0
	0x40 + 0, // display start line = 0
	0x20, 0x00, // horizontal memory addressing mode
	0xa1, // segment remap (a0/a1)
	0xda, 0x12, // com pins configuration
	0x81, 200, // contrast
	0xd9, 0xf1, // pre-charge period
	0xdb, 0x30, // vcomh deselect level
	0xa4, // entire display on
	0xa6, // normal/inverse (a6/a7)
};
uint8_t const displayOn[] = {0xaf};	

uint8_t const displayRegion[] = {
	0x21, 0, 127, // set column start and end address
	0x22, 0, 7 // set page start and end address
};

class Display : public Bitmap {
public:

	Display() : Bitmap(128, 64, data) {}

	template <uint32_t SPI, uint32_t SCK_PIN, uint32_t MOSI_PIN, uint32_t NSS_PIN, uint32_t DMA_CHANNEL>
	void setup(uint32_t resetPin, uint32_t dataPin) {
		this->spi = SPI;
		rccClockEnable<SPI>();

		// configure pins
		Pin().setup<SCK_PIN, SPI, SCK>().setup(Pin::OutputSpeed::_50MHZ);
		Pin().setup<MOSI_PIN, SPI, MOSI>().setup(Pin::OutputSpeed::_50MHZ);
		Pin().setup<NSS_PIN, SPI, NSS>().setup(Pin::OutputSpeed::_50MHZ);
		this->resetPin.setup(resetPin).setup(Pin::OutputSpeed::_50MHZ).clear().setup(Pin::Mode::OUTPUT);
		this->dataPin.setup(dataPin).setup(Pin::OutputSpeed::_50MHZ).clear().setup(Pin::Mode::OUTPUT);

		// configure SPI (1/64 clock, idle high, data valid on 2nd clock pulse, msb first, hardware NSS)
		spi_init_master(SPI, SPI_CR1_BAUDRATE_FPCLK_DIV_64, SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
			SPI_CR1_CPHA_CLK_TRANSITION_2, SPI_CR1_MSBFIRST);

		// setup DMA channel
		this->dmaChannel
			.setup<DMA_CHANNEL, SPI, TX>(DmaChannel::Mode::MEMORY_TO_PERIPHERAL)
			.setup(DmaChannel::Size::_8BIT)
			.setup(DmaChannel::Priority::VERY_HIGH)
			.setup(DmaChannel::Increment::MEMORY)
			.setPeripheralAddress(&SPI_DR(SPI));
		
  		// enable SPI
		spi_enable(SPI);
		spi_enable_tx_dma(SPI);

		// init display controller
		this->resetPin.set();
		send(displayInit, sizeof(displayInit));
		wait();
		
		// clear display
		clear();
		send(displayInit, sizeof(displayRegion));
		wait();
		this->dataPin.set();
		send(this->data, 128*64/8);
		wait();
		this->dataPin.clear();
		
		// switch display on
		send(displayOn, sizeof(displayOn));
		wait();
		
		// set data mode so we start with the first step in update()
		this->dataPin.set();
		
		this->busy = false;
	}

	void update() {
		this->busy = true;
	}
	
	// send data and return true if busy
	bool send() {
		if (this->dmaChannel.isRunning()) {
			// display is busy
			return true;
		}
		if (!this->busy)
			return false;
			
		if (this->dataPin) {
			// set update region
			this->dataPin.clear();
			send(displayInit, sizeof(displayRegion));
		} else {			
			// transfer data
			this->dataPin.set();
			send(this->data, 128*64/8);
			this->busy = false;
		}
		return true;
	}
	
protected:

	void send(uint8_t const * data, size_t size) {
		this->dmaChannel	
			.setMemoryAddress(data)
			.setCount(size)
			.enable();		
	}

	void wait() {
		while (!this->dmaChannel.isComplete());
		this->dmaChannel.resetInterruptFlags();
	}

	uint32_t spi;
	DmaChannel dmaChannel;
	Pin resetPin;
	Pin dataPin;
	uint8_t data[128*64/8];
	
	bool busy;	
};



class LedStrip {
public:
	template <uint32_t USART, uint32_t TX_PIN, uint32_t RX_PIN, uint32_t DMA_CHANNEL>
	void setup() {
		this->usart = USART;
		rccClockEnable<USART>();
		
		// configure pins
		Pin().setup<TX_PIN, USART, TX>().setup(Pin::OutputSpeed::_50MHZ);
		Pin().setup<RX_PIN, USART, RX>().setup(Pin::OutputSpeed::_50MHZ);
		
		// configure USART
		usart_set_baudrate(USART, 4000000);
		usart_set_databits(USART, 8);
		usart_set_parity(USART, USART_PARITY_NONE);
		usart_set_stopbits(USART, USART_STOPBITS_1);
		usart_set_mode(USART, USART_MODE_TX);
		usart_set_flow_control(USART, USART_FLOWCONTROL_NONE);
	
		// setup DMA channel
		this->dmaChannel
			.setup<DMA_CHANNEL, USART, TX>(DmaChannel::Mode::MEMORY_TO_PERIPHERAL)
			.setup(DmaChannel::Size::_8BIT)
			.setup(DmaChannel::Priority::VERY_HIGH)
			.setup(DmaChannel::Increment::MEMORY)
			.setPeripheralAddress(&USART_TDR(USART));

		// enable USART
		usart_enable(USART);
		usart_enable_tx_dma(USART);
		
		// dummy send so that isComplete() of dma channel reports true in send() 
		this->buffer[0] = ledSerialTable[0];
		this->buffer[1] = ledSerialTable[0];
		this->buffer[2] = ledSerialTable[0];
		this->dmaChannel
			.setMemoryAddress(this->buffer)
			.setCount(sizeof(this->buffer))
			.enable();
	}
	
	void send(uint8_t red, uint8_t green, uint8_t blue) {
		// wait until last transmission is complete
		while (!this->dmaChannel.isComplete());
		this->dmaChannel.resetInterruptFlags();
		
		// set color to buffer
		this->buffer[0] = ledSerialTable[green];
		this->buffer[1] = ledSerialTable[red];
		this->buffer[2] = ledSerialTable[blue];
		
		// send
		this->dmaChannel
			.setMemoryAddress(this->buffer)
			.setCount(sizeof(this->buffer))
			.enable();
	}

protected:
	uint32_t usart;
	DmaChannel dmaChannel;
	
	uint32_t buffer[3];
};


Pin debugLed;
Flash flash(112, 16);
Encoder encoder1;
Encoder encoder2;
Button button2;
Display display;
LedStrip strip1;

inline void sendColor(uint8_t red, uint8_t green, uint8_t blue) {
	strip1.send(red, green, blue);
}

inline void sendRGB(RGB color) {
	strip1.send(color.red, color.green, color.blue);
}


// effects (need sendRGB() and sendColor())
#include "Color.h"
#include "Rainbow.h"
#include "MovingLight.h"
#include "Spring.h"
#include "Summer.h"
#include "Autumn.h"
#include "Winter.h"
#include "Plasma.h"
ParameterInfo const brightnessInfo = PARAMETER("Brightness", 25, 255, 8, 255);
EffectInfo const effectInfos[] = {
	EFFECT(Color),
	EFFECT(Rainbow),
	EFFECT(Spring),
	EFFECT(Summer),
	EFFECT(Autumn),
	EFFECT(Winter),
	EFFECT(Plasma),
	EFFECT(MovingLight),
};



int main(void) {
	setupClocks();

	
	debugLed.setup(Pin::PB9).setup(Pin::Mode::OUTPUT).setup(Pin::OutputSpeed::_2MHZ);

	
	//setupDebugLed();
	

	// test
	while (1) {
		//gpio_toggle(DEBUG_LED_PORT, DEBUG_LED_PIN);
		debugLed.toggle();

		// wait
		for (int i = 0; i < 4000000; i++) {
			__asm__("NOP");
		}
		
		// send
		usart_send_blocking(USART1, 0x55);
	}
	
	// constants
	int const effectCount = sizeof(effectInfos) / sizeof(EffectInfo);
	int const maxParameterCount = 8;

	// setup peripherals
	flash.setup();
	encoder1.setup<TIM1, Pin::PA8, Pin::PA9>(effectCount - 1);
	encoder2.setup<TIM2, Pin::PA0, Pin::PA1>();
	//button1.setup(Pin::PC14);
	button2.setup(Pin::PC15);
	display.setup<SPI2, Pin::PB13, Pin::PB15, Pin::PB12, DmaChannel::DMA1_CHANNEL5>(Pin::PB2, Pin::PA10);
	strip1.setup<USART1, Pin::PB6, Pin::PB7, DmaChannel::DMA2_CHANNEL6>();

	// effect index
	encoder1.setValue(flash.read8(0, 0));

	// parameters
	uint8_t brightness = flash.read8(1, 255);
	uint8_t parameters[maxParameterCount];

	// current effect
	EffectInfo const * effectInfo = nullptr;
	uint8_t effectData[32];
	
	// current parameter
	int parameterIndex = 0;

	// main loop
	bool update = false;
	while (1) {

		// check if effect has changed
		int effectIndex = encoder1.getValue();
		if (effectInfo != &effectInfos[effectIndex]) {
			effectInfo = &effectInfos[effectIndex];

			// write effect index to flash
			flash.write8(0, effectIndex);

			// read all effect parameters from flash
			uint16_t offset = 2 + effectIndex * maxParameterCount;
			for (int i = 0; i < effectInfo->parameterCount; ++i)
				parameters[i] = flash.read8(offset + i, effectInfo->parameterInfos[i].initValue);

			// init new effect
			effectInfo->init(effectData, LED_COUNT);

			
			// index of parameter to display
			parameterIndex = 0;
			
			// update display
			update = true;
		}
		
		// check if parameter index has changed
		if (button2.pressed()) {
			++parameterIndex;
			if (parameterIndex >= effectInfo->parameterCount)
				parameterIndex = 0;
				
			// update display
			update = true;
		}
		
		// check if parameter value has changed
		int increment = encoder2.getValue();
		if (increment != 0) {
			encoder2.setValue(0);
			update = false;
			
			// get parameter info and value
			ParameterInfo const *parameterInfo;
			uint8_t *value;
			uint16_t offset;
			if (parameterIndex == 0) {
				// brightness
				parameterInfo = &brightnessInfo;
				value = &brightness;
				offset = 1;
			} else {
				// effect parameter
				parameterInfo = &effectInfo->parameterInfos[parameterIndex - 1];
				value = & parameters[parameterIndex - 1];
				offset = 2 + effectIndex * maxParameterCount;
			}
			
			// increment value
			int newValue = int(value) + increment * int(parameterInfo->step);
			if (newValue < parameterInfo->minValue)
				newValue = parameterInfo->minValue;
			if (newValue > parameterInfo->maxValue)
				newValue = parameterInfo->maxValue;
			*value = uint8_t(newValue);

			// write parameter value to flash
			flash.write8(offset, *value);

			// update display
			update = true;
		}
		
		// display
		if (update && !display.send()) {
			update = false;
			
			// get parameter info and value
			ParameterInfo const * parameterInfo = parameterIndex == 0 ? &brightnessInfo : &effectInfo->parameterInfos[parameterIndex - 1];
			int value = parameterIndex == 0 ? brightness : parameters[parameterIndex - 1];
					
			// clear display
			display.clear();
	
			// effect name
			int y = 10;
			int len = tahoma_8pt.calcWidth(effectInfo->name);
			display.drawText((display.width - len) >> 1, y, tahoma_8pt, effectInfo->name, Bitmap::SET);
			
			// parameter name and value
			y = 30;
			int w = 124 * (value - parameterInfo->minValue) / (parameterInfo->maxValue - parameterInfo->minValue);
			display.fillRectangle(2, y + 2, w, 13, Bitmap::SET);
			len = tahoma_8pt.calcWidth(parameterInfo->name);
			display.drawText(((display.width - len) >> 1), y + 3, tahoma_8pt, parameterInfo->name, Bitmap::FLIP);
			display.drawRectangle(0, y, 128, 17, Bitmap::SET);
			
			// update display
			display.update();
		}

		// run effect
		effectInfo->run(effectData, LED_COUNT, brightness, parameters);

		// wait 50us		
	}

	return 0;
}
