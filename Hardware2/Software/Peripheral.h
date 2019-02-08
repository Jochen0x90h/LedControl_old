#pragma once

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/adc.h>


enum Function {
	// TIMx
	CH1 = 1,
	CH1N,
	CH2,
	CH2N,
	CH3,
	CH3N,
	CH4,
	BKIN,
	BKIN2,
	ETR,
	
	// SPIx
	SCK,
	MISO,
	MOSI,
	NSS,
	
	// USARTx
	CK,
	RX,
	TX,
	RTS_DE,
	CTS
};


// compile time key-value store for peripheral reset and clock control register (RCC) flags
// https://stackoverflow.com/questions/16490835/how-to-build-a-compile-time-key-value-store
template <uint32_t K, rcc_periph_clken V>
struct RccItem {};

template <typename...>
struct RccMap;

template <>
struct RccMap<>
{
	template <uint32_t k>
	struct get
	{
		static const rcc_periph_clken value = {};
		static const bool exists = false;
	};
};

template<uint32_t K, rcc_periph_clken V, typename... rest>
struct RccMap<RccItem<K, V>, rest...>
{
	template <uint32_t k>
	struct get
	{
		static const rcc_periph_clken value = (k == K) ? V : RccMap<rest...>::template get<k>::value;
		static const bool exists = (k == K) || RccMap<rest...>::template get<k>::exists;
	};
};

using rccMap = RccMap<
	RccItem<DMA1, RCC_DMA1>,
	RccItem<DMA2, RCC_DMA2>,
	RccItem<TIM1, RCC_TIM1>,
	RccItem<TIM2, RCC_TIM2>,
	RccItem<SPI1, RCC_SPI1>,
	RccItem<SPI2, RCC_SPI2>,
	RccItem<SPI3, RCC_SPI3>,
	RccItem<USART1, RCC_USART1>,
	RccItem<USART2, RCC_USART2>,
	RccItem<USART3, RCC_USART3>
>;
/*
template <uint32_t k>
rcc_periph_clken getRcc() {
	static_assert(rccMap::get<k>::exists, "RCC not found");
	return rccMap::get<k>::value;
}
*/
template <uint32_t k>
void rccClockEnable() {
	static_assert(rccMap::get<k>::exists, "RCC not found");
	rcc_periph_clock_enable(rccMap::get<k>::value);
}
