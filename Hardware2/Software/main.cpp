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


// STM32L4
// Data Sheet: https://www.st.com/resource/en/datasheet/stm32l433cc.pdf
//   Page 74: Alternative functions for io pins
// Reference Manual: https://www.st.com/content/ccc/resource/technical/document/reference_manual/group0/b0/ac/3e/8f/6d/21/47/af/DM00151940/files/DM00151940.pdf/jcr:content/translations/en.DM00151940.pdf
//   Page 295: DMA
//   Page 716: Timer encoder interface mode



/*
// helper functions
void gpio_set_input(uint32_t gpioport, uint16_t gpios, uint8_t pull_up_down) {
	gpio_mode_setup(gpioport, GPIO_MODE_INPUT, pull_up_down, gpios);
}

void gpio_set_input_af(uint32_t gpioport, uint16_t gpios, uint8_t pull_up_down, uint8_t alt_func_num) {
	gpio_mode_setup(gpioport, GPIO_MODE_AF, pull_up_down, gpios);
	gpio_set_af(gpioport, alt_func_num, gpios);
}

void gpio_set_output(uint32_t gpioport, uint16_t gpios, uint8_t otype, uint8_t speed) {
	gpio_mode_setup(gpioport, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, gpios);
	gpio_set_output_options(gpioport, otype, speed, gpios);
}

void gpio_set_output_af(uint32_t gpioport, uint16_t gpios, uint8_t otype, uint8_t speed, uint8_t alt_func_num) {
	gpio_mode_setup(gpioport, GPIO_MODE_AF, GPIO_PUPD_NONE, gpios);
	gpio_set_output_options(gpioport, otype, speed, gpios);
	gpio_set_af(gpioport, alt_func_num, gpios);
}
*/

// clocks
void setupClocks() {
	// enbable HSI16 for SYSCLK and HSI48 for USB
	rcc_osc_on(RCC_HSI16);
	//rcc_osc_on(RCC_HSI48);
	
	// configure flash
	flash_prefetch_enable();
	flash_set_ws(4);
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
	gpio_set_output(DEBUG_LED_PORT, DEBUG_LED_PIN, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ);
}*/


class Encoder {
public:
	template <uint32_t TIMER, uint32_t CH1_PIN, uint32_t CH2_PIN>
	void setup(uint32_t buttonPin) {
		this->timer = TIMER;
		rcc_periph_clock_enable(getRcc<TIMER>());

		// configure pins
		Pin().setup<CH1_PIN, TIMER, CH1>().setup(Pin::Resistor::PULLDOWN);
		Pin().setup<CH2_PIN, TIMER, CH2>().setup(Pin::Resistor::PULLDOWN);
		this->button.setup(buttonPin).setup(Pin::Mode::INPUT).setup(Pin::Resistor::PULLDOWN);
		
		// configure timer
		timer_set_period(this->timer, 0xffffffff);
		timer_slave_set_mode(this->timer, TIM_SMCR_SMS_EM3); // encoder mode
		timer_ic_set_input(this->timer, TIM_IC1, TIM_IC_IN_TI1); // channel 1 on input 1
		timer_ic_set_input(this->timer, TIM_IC2, TIM_IC_IN_TI2); // channel 2 on input 2
	
		// enable timer
		timer_enable_counter(this->timer);
	}

	int getValue() {
		return timer_get_counter(this->timer);
	}

	bool getButtonState() {
		return this->button;
	}

protected:
	uint32_t timer;
	Pin button;
};

uint8_t const displayInit[] = {
	0xae, // display off
	0xd5, 0x80, // clock divide ratio and oscillator frequency
	0xa8, 63, // multiplex ratio = 1/64
	0xc8, // output scan direction, vertical flip (c0/c8)
	0xd3, 0, // display offset = 0
	0x40 + 0, // display start line = 0
	0x20, 0x00, // horizontal addressing mode
	0xa1, // segment remap (a0/a1)
	0xda, 0x12, // com pins configuration
	0x81, 200, // contrast
	0xd9, 0xf1, // pre-charge period
	0xdb, 0x30, // vcomh deselect level
	0xa4, // entire display on
	0xa6, // normal/inverse (a6/a7)
};
uint8_t const displayOn[] = {0xaf};	

class Display {
public:

	template <uint32_t SPI, uint32_t SCK_PIN, uint32_t MOSI_PIN, uint32_t NSS_PIN, uint32_t DMA_CHANNEL>
	void setup(uint32_t resetPin) {
		this->spi = SPI;
		rcc_periph_clock_enable(getRcc<SPI>());

		// configure pins
		Pin().setup<SCK_PIN, SPI, SCK>().setup(Pin::OutputSpeed::_50MHZ);
		Pin().setup<MOSI_PIN, SPI, MOSI>().setup(Pin::OutputSpeed::_50MHZ);
		Pin().setup<NSS_PIN, SPI, NSS>().setup(Pin::OutputSpeed::_50MHZ);
		this->reset.setup(resetPin).setup(Pin::OutputSpeed::_50MHZ).clear().setup(Pin::Mode::OUTPUT);

		// configure SPI (1/64 clock, idle high, data valid on 2nd clock pulse, msb first, hardware NSS)
		spi_init_master(this->spi, SPI_CR1_BAUDRATE_FPCLK_DIV_64, SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
			SPI_CR1_CPHA_CLK_TRANSITION_2, SPI_CR1_MSBFIRST);

  		// enable SPI
		spi_enable(this->spi);

		// setup and enable DMA channel
		this->dmaChannel
			.setup<DMA_CHANNEL, SPI, TX>(DmaChannel::Mode::MEMORY_TO_PERIPHERAL)
			.setup(DmaChannel::Size::_8BIT)
			.setup(DmaChannel::Priority::VERY_HIGH)
			.setup(DmaChannel::Increment::MEMORY);
			//.setPeripheralAddress();			
		

		// init display controller
		this->reset.set();
		send(displayInit, sizeof(displayInit));
		//clear();
		update();
		send(displayOn, sizeof(displayOn));
	}

	void update() {
	
	}

protected:

	void send(uint8_t const * data, size_t size) {
		this->dmaChannel	
			.setMemoryAddress(data)
			.setCount(size)
			.enable();
		
	}

	uint32_t spi;
	DmaChannel dmaChannel;
	Pin reset;
	uint8_t data[128*64/8];
};





// LED strip 1
#define STRIP1_TX_PORT GPIOB
#define STRIP1_TX_PIN GPIO6
#define STRIP1_RX_PORT GPIOB
#define STRIP1_RX_PIN GPIO7

void setupStrip1() {
/*	rcc_periph_clock_enable(RCC_USART1);

	// configure IOs
	gpio_set_output_af(STRIP1_TX_PORT, STRIP1_TX_PIN, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_AF7_USART1);
	gpio_set_input_af(STRIP1_RX_PORT, STRIP1_RX_PIN, GPIO_PUPD_PULLUP, GPIO_AF7_USART1);

	// configure USART
	usart_set_baudrate(USART1, 4000000);
	usart_set_databits(USART1, 8);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	// enable USART
	usart_enable(USART1);
	
	// dma, see https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f1/lisa-m-2/usart_dma/usart_dma.c
	nvic_set_priority(NVIC_DMA1_CHANNEL7_IRQ, 0);
	nvic_enable_irq(NVIC_DMA1_CHANNEL7_IRQ);
*/
}

volatile int strip1Busy = 0;
uint32_t strip1Buffer[3];

void sendStrip1() {
	strip1Busy = 1;
	
	dma_channel_reset(DMA1, DMA_CHANNEL7);

	dma_set_peripheral_address(DMA1, DMA_CHANNEL7, (uint32_t)&USART2_TDR);
	dma_set_peripheral_size(DMA1, DMA_CHANNEL7, DMA_CCR_PSIZE_8BIT);
	dma_set_memory_size(DMA1, DMA_CHANNEL7, DMA_CCR_MSIZE_8BIT);
	dma_set_read_from_memory(DMA1, DMA_CHANNEL7);
	dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL7);
	dma_set_priority(DMA1, DMA_CHANNEL7, DMA_CCR_PL_VERY_HIGH);

	dma_set_memory_address(DMA1, DMA_CHANNEL7, (uint32_t)strip1Buffer);
	dma_set_number_of_data(DMA1, DMA_CHANNEL7, sizeof(strip1Buffer));

	dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL7);
	dma_enable_channel(DMA1, DMA_CHANNEL7);
	usart_enable_tx_dma(USART1);
}

void dma1_channel7_isr(void)
{
	if ((DMA1_ISR & DMA_ISR_TCIF7) != 0) {
		DMA1_IFCR |= DMA_IFCR_CTCIF7;
		strip1Busy = 0;
	}

	dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL7);
	usart_disable_tx_dma(USART1);
	dma_disable_channel(DMA1, DMA_CHANNEL7);
}

void sendColor(uint8_t red, uint8_t green, uint8_t blue) {
	// wait until last transmission is complete
	while (strip1Busy);
	
	strip1Buffer[0] = ledSerialTable[green];
	strip1Buffer[1] = ledSerialTable[red];
	strip1Buffer[2] = ledSerialTable[blue];
	
	sendStrip1();
}

inline void sendRGB(RGB color) {
	sendColor(color.red, color.green, color.blue);
}

Pin debugLed;
Encoder encoder1;
Encoder encoder2;
Display display;

int main(void) {
	setupClocks();
	
	debugLed.setup(Pin::PB9).setup(Pin::Mode::OUTPUT).setup(Pin::OutputSpeed::_2MHZ);

	encoder1.setup<TIM1, Pin::PA8, Pin::PA9>(Pin::PC14);
	encoder2.setup<TIM2, Pin::PA0, Pin::PA1>(Pin::PC15);
	display.setup<SPI2, Pin::PB13, Pin::PB15, Pin::PB12, DmaChannel::DMA1_CHANNEL5>(Pin::PB2);

	//setupDebugLed();
	setupStrip1();

	//Bitmap display(128, 64, displayData);
	

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


	return 0;
}
