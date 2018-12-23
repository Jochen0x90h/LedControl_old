#pragma once

#include <libopencm3/stm32/gpio.h>
#include "Peripheral.h"


class Pin {
public:
	static const int PA0 = GPIOA;
	static const int PA1 = GPIOA + 1;
	static const int PA2 = GPIOA + 2;
	static const int PA3 = GPIOA + 3;
	static const int PA4 = GPIOA + 4;
	static const int PA5 = GPIOA + 5;
	static const int PA6 = GPIOA + 6;
	static const int PA7 = GPIOA + 7;
	static const int PA8 = GPIOA + 8;
	static const int PA9 = GPIOA + 9;
	static const int PA10 = GPIOA + 10;
	static const int PA11 = GPIOA + 11;
	static const int PA12 = GPIOA + 12;
	static const int PA13 = GPIOA + 13;
	static const int PA14 = GPIOA + 14;
	static const int PA15 = GPIOA + 15;

	static const int PB0 = GPIOB;
	static const int PB1 = GPIOB + 1;
	static const int PB2 = GPIOB + 2;
	static const int PB3 = GPIOB + 3;
	static const int PB4 = GPIOB + 4;
	static const int PB5 = GPIOB + 5;
	static const int PB6 = GPIOB + 6;
	static const int PB7 = GPIOB + 7;
	static const int PB8 = GPIOB + 8;
	static const int PB9 = GPIOB + 9;
	static const int PB10 = GPIOB + 10;
	static const int PB11 = GPIOB + 11;
	static const int PB12 = GPIOB + 12;
	static const int PB13 = GPIOB + 13;
	static const int PB14 = GPIOB + 14;
	static const int PB15 = GPIOB + 15;

	static const int PC13 = GPIOC + 13;
	static const int PC14 = GPIOC + 14;
	static const int PC15 = GPIOC + 15;


	enum class Mode {
		INPUT = 0,
		OUTPUT = 1,
		AF = 2,
		ANALOG = 3
	};

	enum class Resistor {
		NONE = 0,
		PULLUP = 1,
		PULLDOWN = 2
	};
	
	enum class OutputSpeed {
		// 2MHz
		_2MHZ = 0,
	
		// 25MHz
		_25MHZ = 1,
	
		// 50MHz
		_50MHZ = 2,
	
		// 100 MHz
		_100MHZ = 3
	};
	
	enum class OutputDriver {
		PUSH_PULL = 0,
		OPEN_DRAIN = 1
	};

	enum class AlternateFunction {
		AF0 = 0,
		AF1 = 1,
		AF2 = 2,
		AF3 = 3,
		AF4 = 4,
		AF5 = 5,
		AF6 = 6,
		AF7 = 7,
		AF8 = 8,
		AF9 = 9,
		AF10 = 10,
		AF11 = 11,
		AF12 = 12,
		AF13 = 13,
		AF14 = 14,
		AF15 = 15
	};


	Pin & setup(uint32_t pin) {
		this->pin = pin;
		return *this;
	}
	
	Pin & setup(Mode mode) {
		uint32_t port = this->pin & ~0xf;
		int index = (this->pin & 0xf) * 2;
		GPIO_MODER(port) = (GPIO_MODER(port) & ~(3 << index)) | (int(mode) << index);
		return *this;
	}
	
	/**
	 * Setup pin for alternate peripheral function. Produces a compile error if the pin does not support the function
	 */
	template <uint32_t PIN, uint32_t PERIPHERAL, Function FUNCTION>
	Pin & setup();

	Pin & setup(Resistor resistor) {
		uint32_t port = this->pin & ~0xf;
		int index = (this->pin & 0xf) * 2;
		GPIO_PUPDR(port) = (GPIO_PUPDR(port) & ~(3 << index)) | (int(resistor) << index);
		return *this;
	}

	Pin & setup(OutputDriver outputDriver) {
		uint32_t port = this->pin & ~0xf;
		int index = this->pin & 0xf;
		GPIO_OTYPER(port) = (GPIO_OTYPER(port) & ~(1 << index)) | (int(outputDriver) << index);
		return *this;
	}

	Pin & setup(OutputSpeed outputSpeed) {
		uint32_t port = this->pin & ~0xf;
		int index = (this->pin & 0xf) * 2;
		GPIO_OSPEEDR(port) = (GPIO_OSPEEDR(port) & ~(3 << index)) | (int(outputSpeed) << index);
		return *this;
	}
	
	bool operator =(bool value) {
		uint32_t port = this->pin & ~0xf;
		uint32_t bit = (value ? 1 : 0x10000) << (this->pin & 0xf);
		GPIO_BSRR(port) = bit;
		return value;	
	}

	operator bool () {
		uint32_t port = this->pin & ~0xf;
		uint32_t bit = 1 << (this->pin & 0xf);
		return (GPIO_IDR(port) & bit) != 0;
	}

	Pin & set() {
		uint32_t port = this->pin & ~0xf;
		uint32_t bit = 1 << (this->pin & 0xf);
		GPIO_BSRR(port) = bit;
		return *this;
	}

	Pin & clear() {
		uint32_t port = this->pin & ~0xf;
		uint32_t bit = 0x10000 << (this->pin & 0xf);
		GPIO_BSRR(port) = bit;
		return *this;
	}

	Pin & toggle() {
		uint32_t port = this->pin & ~0xf;
		uint32_t bit = 1 << (this->pin & 0xf);
		uint32_t odr = GPIO_ODR(port);
		GPIO_BSRR(port) = ((odr & bit) << 16) | (~odr & bit);
		return *this;
	}

protected:
	uint32_t pin;
};



// compile time key-value store for GPIO alternate function
template <uint32_t K1, uint32_t K2, int K3, Pin::AlternateFunction V>
struct AfItem {};

template <typename...>
struct AfMap;

template <>
struct AfMap<>
{
	template <uint32_t k1, uint32_t k2, int k3>
	struct get
	{
		static const Pin::AlternateFunction value = {};
		static const bool exists = false;
	};
};

template<uint32_t K1, uint32_t K2, int K3, Pin::AlternateFunction V, typename... rest>
struct AfMap<AfItem<K1, K2, K3, V>, rest...>
{
	template <uint32_t k1, uint32_t k2, int k3>
	struct get
	{
		static const Pin::AlternateFunction value = (k1 == K1 && k2 == K2 && k3 == K3) ? V : AfMap<rest...>::template get<k1, k2, k3>::value;
		static const bool exists = (k1 == K1 && k2 == K2 && k3 == K3) || AfMap<rest...>::template get<k1, k2, k3>::exists;
	};
};

/**
 * STM32L433xx
 * Ultra-low-power Arm Cortex-M4 32-bit MCU+FPU
 * Page 74
 */
using afMap = AfMap<
	// Table 16. Alternate function AF0 to AF7
	AfItem<Pin::PA0,  TIM2, CH1,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA1,  TIM2, CH2,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA2,  TIM2, CH3,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA3,  TIM2, CH4,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA5,  TIM2, CH1,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA6,  TIM1, BKIN, Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA7,  TIM1, CH1N, Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA8,  TIM1, CH1,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA9,  TIM1, CH2,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA10, TIM1, CH3,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA11, TIM1, CH4,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA12, TIM1, ETR,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PA15, TIM2, CH1,  Pin::AlternateFunction::AF1>,

	AfItem<Pin::PB0,  TIM1, CH2N, Pin::AlternateFunction::AF1>,
	AfItem<Pin::PB1,  TIM1, CH3N, Pin::AlternateFunction::AF1>,
	AfItem<Pin::PB3,  TIM2, CH2,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PB10, TIM2, CH3,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PB11, TIM2, CH4,  Pin::AlternateFunction::AF1>,
	AfItem<Pin::PB12, TIM1, BKIN, Pin::AlternateFunction::AF1>,
	AfItem<Pin::PB13, TIM1, CH1N, Pin::AlternateFunction::AF1>,
	AfItem<Pin::PB14, TIM1, CH2N, Pin::AlternateFunction::AF1>,
	AfItem<Pin::PB15, TIM1, CH3N, Pin::AlternateFunction::AF1>,

	AfItem<Pin::PA5,  TIM2, ETR,   Pin::AlternateFunction::AF2>,
	AfItem<Pin::PA11, TIM1, BKIN2, Pin::AlternateFunction::AF2>,
	AfItem<Pin::PA15, TIM2, ETR,   Pin::AlternateFunction::AF2>,

	AfItem<Pin::PA1,  SPI1, SCK,  Pin::AlternateFunction::AF5>,
	AfItem<Pin::PA4,  SPI1, NSS,  Pin::AlternateFunction::AF5>,
	AfItem<Pin::PA5,  SPI1, SCK,  Pin::AlternateFunction::AF5>,
	AfItem<Pin::PA6,  SPI1, MISO, Pin::AlternateFunction::AF5>,
	AfItem<Pin::PA7,  SPI1, MOSI, Pin::AlternateFunction::AF5>,
	AfItem<Pin::PA11, SPI1, MISO, Pin::AlternateFunction::AF5>,
	AfItem<Pin::PA12, SPI1, MOSI, Pin::AlternateFunction::AF5>,
	AfItem<Pin::PA15, SPI1, NSS,  Pin::AlternateFunction::AF5>,

	AfItem<Pin::PB0,  SPI1, NSS,  Pin::AlternateFunction::AF5>,
	AfItem<Pin::PB3,  SPI1, SCK,  Pin::AlternateFunction::AF5>,
	AfItem<Pin::PB4,  SPI1, MISO, Pin::AlternateFunction::AF5>,
	AfItem<Pin::PB5,  SPI1, MOSI, Pin::AlternateFunction::AF5>,
	AfItem<Pin::PB9,  SPI2, NSS,  Pin::AlternateFunction::AF5>,
	AfItem<Pin::PB10, SPI2, SCK,  Pin::AlternateFunction::AF5>,
	AfItem<Pin::PB12, SPI2, NSS,  Pin::AlternateFunction::AF5>,
	AfItem<Pin::PB13, SPI2, SCK,  Pin::AlternateFunction::AF5>,
	AfItem<Pin::PB14, SPI2, MISO, Pin::AlternateFunction::AF5>,
	AfItem<Pin::PB15, SPI2, MOSI, Pin::AlternateFunction::AF5>,

	AfItem<Pin::PA4,  SPI3, NSS,  Pin::AlternateFunction::AF6>,
	AfItem<Pin::PA15, SPI3, NSS,  Pin::AlternateFunction::AF6>,

	AfItem<Pin::PB3,  SPI3, SCK,  Pin::AlternateFunction::AF6>,
	AfItem<Pin::PB4,  SPI3, MISO, Pin::AlternateFunction::AF6>,
	AfItem<Pin::PB5,  SPI3, MOSI, Pin::AlternateFunction::AF6>,

	AfItem<Pin::PA0,  USART2, CTS,    Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA1,  USART2, RTS_DE, Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA2,  USART2, TX,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA3,  USART2, RX,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA4,  USART2, CK,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA6,  USART3, CTS,    Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA8,  USART1, CK,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA9,  USART1, TX,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA10, USART1, RX,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA11, USART1, CTS,    Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA12, USART1, RTS_DE, Pin::AlternateFunction::AF7>,
	AfItem<Pin::PA15, USART3, RTS_DE, Pin::AlternateFunction::AF7>,

	AfItem<Pin::PB0,  USART3, CK,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB1,  USART3, RTS_DE, Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB3,  USART1, RTS_DE, Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB4,  USART1, CTS,    Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB5,  USART1, CK,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB6,  USART1, TX,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB7,  USART1, RX,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB10, USART3, TX,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB11, USART3, RX,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB12, USART3, CK,     Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB13, USART3, CTS,    Pin::AlternateFunction::AF7>,
	AfItem<Pin::PB14, USART3, RTS_DE, Pin::AlternateFunction::AF7>,

	// Table 17. Alternate function AF8 to AF15
	AfItem<Pin::PA0,  TIM2, ETR,   Pin::AlternateFunction::AF14>
>;

template <uint32_t PIN, uint32_t PERIPHERAL, Function FUNCTION>
Pin & Pin::setup() {
	this->pin = PIN;
	uint32_t port = PIN & ~0xf;
	int index = (PIN & 0xf) * 4;
	
	static_assert(afMap::get<PIN, PERIPHERAL, FUNCTION>::exists, "Alternate Function not found");
	AlternateFunction alternateFunction = afMap::get<PIN, PERIPHERAL, FUNCTION>::value;
	if (index < 32) {
		GPIO_AFRL(port) = (GPIO_AFRL(port) & ~(15 << index)) | (int(alternateFunction) << index);		
	} else {
		index &= 31;
		GPIO_AFRH(port) = (GPIO_AFRH(port) & ~(15 << index)) | (int(alternateFunction) << index);				
	}		
	return setup(Mode::AF);
}
