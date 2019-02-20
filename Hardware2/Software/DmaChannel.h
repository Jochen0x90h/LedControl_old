#pragma once

#include <libopencm3/stm32/dma.h>
#include "Peripheral.h"


/* DMA channel selection register (DMAx_CSELR) */
 #define DMA_CSELR(dma_base)             MMIO32((dma_base) + 0xA8)
 #define DMA1_CSELR                      DMA_CSELR(DMA1)
 #define DMA2_CSELR                      DMA_CSELR(DMA2)
 
/**
 * DMA channel
 * Reference Manual, Page 295: https://www.st.com/content/ccc/resource/technical/document/reference_manual/group0/b0/ac/3e/8f/6d/21/47/af/DM00151940/files/DM00151940.pdf/jcr:content/translations/en.DM00151940.pdf
 * example usage: https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f1/lisa-m-2/usart_dma/usart_dma.c
*/	
class DmaChannel {
public:
	static const int DMA1_CHANNEL1 = DMA1 + DMA_CHANNEL1;
	static const int DMA1_CHANNEL2 = DMA1 + DMA_CHANNEL2;
	static const int DMA1_CHANNEL3 = DMA1 + DMA_CHANNEL3;
	static const int DMA1_CHANNEL4 = DMA1 + DMA_CHANNEL4;
	static const int DMA1_CHANNEL5 = DMA1 + DMA_CHANNEL5;
	static const int DMA1_CHANNEL6 = DMA1 + DMA_CHANNEL6;
	static const int DMA1_CHANNEL7 = DMA1 + DMA_CHANNEL7;

	static const int DMA2_CHANNEL1 = DMA2 + DMA_CHANNEL1;
	static const int DMA2_CHANNEL2 = DMA2 + DMA_CHANNEL2;
	static const int DMA2_CHANNEL3 = DMA2 + DMA_CHANNEL3;
	static const int DMA2_CHANNEL4 = DMA2 + DMA_CHANNEL4;
	static const int DMA2_CHANNEL5 = DMA2 + DMA_CHANNEL5;
	static const int DMA2_CHANNEL6 = DMA2 + DMA_CHANNEL6;
	static const int DMA2_CHANNEL7 = DMA2 + DMA_CHANNEL7;


	enum class Mode {
		// transfer from peripheral to memory
		PERIPHERAL_TO_MEMORY = 0x0000,
		
		// transfer from memory to peripheral
		MEMORY_TO_PERIPHERAL = 0x0010,

		// circular transfer from peripheral to memory (restart at end)
		CIRCULAR_PERIPHERAL_TO_MEMORY = 0x0002,
		
		// circular transfer from memory to peripheral
		CIRCULAR_MEMORY_TO_PERIPHERAL = 0x0012,
	};

	enum class Size {
		// both memory and peripheral are 8 bit
		_8BIT = 0x0000,

		// both memory and peripheral are 16 bit
		_16BIT = 0x0500,

		// both memory and peripheral are 32 bit
		_32BIT = 0x0A00
	};
	
	enum class Priority {
		LOW = 0x0000,
		MEDIUM = 0x1000,
		HIGH = 0x2000,
		VERY_HIGH = 0x3000
	};
	
	enum class Increment {
		NONE = 0x0000,
		PERIPHERAL = 0x0040,
		MEMORY = 0x0080,
		BOTH = 0x00C0
	};
		
	enum class Interrupt {
		
	};
	
	/**
	 * Setup dma channel for memory to memory transfer where peripheral address is source memory address.
	 * Node: Only call when channel is not enabled
	 */
	template <uint32_t DMA_CHANNEL>
	DmaChannel & setup();
	
	/**
	 * Setup dma channel for use with a peripheral function. Produces a compile error if the dma channel does not support the function
	 * Node: Only call when channel is not enabled
	 */
	template <uint32_t DMA_CHANNEL, uint32_t PERIPHERAL, Function FUNCTION = 0>
	DmaChannel & setup(Mode mode);

	DmaChannel & setup(Size size) {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;		
		DMA_CCR(dma, channel) = (DMA_CCR(dma, channel) & ~0x0F00) | int(size);
		return *this;
	}

	DmaChannel & setup(Priority priority) {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;		
		DMA_CCR(dma, channel) = (DMA_CCR(dma, channel) & ~0x3000) | int(priority);
		return *this;
	}

	DmaChannel & setup(Increment increment) {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;		
		DMA_CCR(dma, channel) = (DMA_CCR(dma, channel) & ~0x00C0) | int(increment);
		return *this;
	}
	
	DmaChannel & setPeripheralAddress(volatile const void * address) {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;
		DMA_CPAR(dma, channel) = (uint32_t)address;
		return *this;
	}

	DmaChannel & setMemoryAddress(const void * address) {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;
		DMA_CMAR(dma, channel) = (uint32_t)address;
		return *this;
	}

	DmaChannel & setCount(uint16_t count) {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;
		DMA_CNDTR(dma, channel) = count;
		return *this;
	}
	
	uint16_t getCount() {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;
		return DMA_CNDTR(dma, channel);
	}
	
	DmaChannel & enable() {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;		
		DMA_CCR(dma, channel) = DMA_CCR(dma, channel) | 0x0001;
		return *this;
	}

	DmaChannel & disable() {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;		
		DMA_CCR(dma, channel) = DMA_CCR(dma, channel) & ~0x0001;
		return *this;
	}

	bool isEnabled() {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;		
		return (DMA_CCR(dma, channel) & 0x0001) != 0;
	}

	/**
	 * Returns true if a DMA transfer is running
	 */
	bool isRunning() {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;		
		return DMA_CNDTR(dma, channel) > 0;
	}
	
	/**
	 * Returns true if a DMA transfer is complete
	 */
	bool isComplete() {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;		
		return (DMA_ISR(dma) & (2 << (channel - 1) * 4)) != 0;
	}

	/**
	 * Reset all interrupt flags (transfer error, half transfer, transfer complete)
	 */
	void resetInterruptFlags() {
		uint32_t dma = this->dmaChannel & ~0x7;
		int channel = this->dmaChannel & 0x7;		
		DMA_IFCR(dma) = 1 << (channel - 1) * 4;
	}

protected:
	uint32_t dmaChannel;
};


// compile time key-value store for dma interrupt vector numbers
// https://stackoverflow.com/questions/16490835/how-to-build-a-compile-time-key-value-store
template <uint32_t K1, uint32_t K2, int K3, int V>
struct DmaSelectItem {};

template <typename...>
struct DmaSelectMap;

template <>
struct DmaSelectMap<>
{
	template <uint32_t k1, uint32_t k2, int k3>
	struct get
	{
		static const int value = {};
		static const bool exists = false;
	};
};

template<uint32_t K1, uint32_t K2, int K3, int V, typename... rest>
struct DmaSelectMap<DmaSelectItem<K1, K2, K3, V>, rest...>
{
	template <uint32_t k1, uint32_t k2, int k3>
	struct get
	{
		static const int value = (k1 == K1 && k2 == K2 && k3 == K3) ? V : DmaSelectMap<rest...>::template get<k1, k2, k3>::value;
		static const bool exists = (k1 == K1 && k2 == K2 && k3 == K3) || DmaSelectMap<rest...>::template get<k1, k2, k3>::exists;
	};
};

/**
 * DMA channel to peripheral function mapping
 * Reference manual: Page 298
 */
using dmaSelectMap = DmaSelectMap<
	// Table 41. DMA1 requests for each channel
	DmaSelectItem<DmaChannel::DMA1_CHANNEL1, ADC1, 0, 0>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL2, SPI1, RX, 1>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL3, SPI1, TX, 1>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL4, SPI2, RX, 1>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL5, SPI2, TX, 1>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL2, USART3, TX, 2>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL3, USART3, RX, 2>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL4, USART1, TX, 2>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL5, USART1, RX, 2>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL6, USART2, RX, 2>,
	DmaSelectItem<DmaChannel::DMA1_CHANNEL7, USART2, TX, 2>,

	// Table 42. DMA2 requests for each channel
	DmaSelectItem<DmaChannel::DMA2_CHANNEL6, USART1, TX, 2>,
	DmaSelectItem<DmaChannel::DMA2_CHANNEL7, USART1, RX, 2>,
	DmaSelectItem<DmaChannel::DMA2_CHANNEL1, SPI3, RX, 3>,
	DmaSelectItem<DmaChannel::DMA2_CHANNEL2, SPI3, TX, 3>,
	DmaSelectItem<DmaChannel::DMA2_CHANNEL3, SPI1, RX, 4>,
	DmaSelectItem<DmaChannel::DMA2_CHANNEL4, SPI1, TX, 4>
>;


// compile time key-value store for dma interrupt vector numbers
// https://stackoverflow.com/questions/16490835/how-to-build-a-compile-time-key-value-store
template <uint32_t K, int V>
struct DmaIrqItem {};

template <typename...>
struct DmaIrqMap;

template <>
struct DmaIrqMap<>
{
	template <uint32_t k>
	struct get
	{
		static const int value = {};
		static const bool exists = false;
	};
};

template<uint32_t K, int V, typename... rest>
struct DmaIrqMap<DmaIrqItem<K, V>, rest...>
{
	template <uint32_t k>
	struct get
	{
		static const int value = (k == K) ? V : DmaIrqMap<rest...>::template get<k>::value;
		static const bool exists = (k == K) || DmaIrqMap<rest...>::template get<k>::exists;
	};
};

using dmaIrqMap = DmaIrqMap<
	DmaIrqItem<DmaChannel::DMA1_CHANNEL1, NVIC_DMA1_CHANNEL1_IRQ>,
	DmaIrqItem<DmaChannel::DMA1_CHANNEL2, NVIC_DMA1_CHANNEL2_IRQ>,
	DmaIrqItem<DmaChannel::DMA1_CHANNEL3, NVIC_DMA1_CHANNEL3_IRQ>,
	DmaIrqItem<DmaChannel::DMA1_CHANNEL4, NVIC_DMA1_CHANNEL4_IRQ>,
	DmaIrqItem<DmaChannel::DMA1_CHANNEL5, NVIC_DMA1_CHANNEL5_IRQ>,
	DmaIrqItem<DmaChannel::DMA1_CHANNEL6, NVIC_DMA1_CHANNEL6_IRQ>,
	DmaIrqItem<DmaChannel::DMA1_CHANNEL7, NVIC_DMA1_CHANNEL7_IRQ>,
	DmaIrqItem<DmaChannel::DMA2_CHANNEL1, NVIC_DMA2_CHANNEL1_IRQ>,
	DmaIrqItem<DmaChannel::DMA2_CHANNEL2, NVIC_DMA2_CHANNEL2_IRQ>,
	DmaIrqItem<DmaChannel::DMA2_CHANNEL3, NVIC_DMA2_CHANNEL3_IRQ>,
	DmaIrqItem<DmaChannel::DMA2_CHANNEL4, NVIC_DMA2_CHANNEL4_IRQ>,
	DmaIrqItem<DmaChannel::DMA2_CHANNEL5, NVIC_DMA2_CHANNEL5_IRQ>,
	DmaIrqItem<DmaChannel::DMA2_CHANNEL6, NVIC_DMA2_CHANNEL6_IRQ>,
	DmaIrqItem<DmaChannel::DMA2_CHANNEL7, NVIC_DMA2_CHANNEL7_IRQ>
>;

template <uint32_t DMA_CHANNEL>
DmaChannel & DmaChannel::setup() {
	this->dmaChannel = DMA_CHANNEL;
	uint32_t dma = DMA_CHANNEL & ~0x7;
	int channel = DMA_CHANNEL & 0x7;		

	// get irq number (for nvic_enable_irq())
	static_assert(dmaIrqMap::get<DMA_CHANNEL>::exists, "DMA IRQ not found");
	//int irq = dmaIrqMap::get<DMA_CHANNEL>::value;

	// set to MEM2MEM mode
	DMA_CCR(dma, channel) = (DMA_CCR(dma, channel) & ~0x4012) | 0x4000;
	return *this;
}

template <uint32_t DMA_CHANNEL, uint32_t PERIPHERAL, Function FUNCTION>
DmaChannel & DmaChannel::setup(Mode mode) {
	this->dmaChannel = DMA_CHANNEL;
	uint32_t dma = DMA_CHANNEL & ~0x7;
	int channel = DMA_CHANNEL & 0x7;		
	
	// select peripheral function
	static_assert(dmaSelectMap::get<DMA_CHANNEL, PERIPHERAL, FUNCTION>::exists, "DMA channel does not support peripheral function");
	int select = dmaSelectMap::get<DMA_CHANNEL, PERIPHERAL, FUNCTION>::value;
	int index = (channel - 1) * 4;
	DMA_CSELR(dma) = (DMA_CSELR(dma) & ~(3 << index)) | (select << index);
		
	// get irq number (for nvic_enable_irq())
	static_assert(dmaIrqMap::get<DMA_CHANNEL>::exists, "DMA IRQ not found");
	//int irq = dmaIrqMap::get<DMA_CHANNEL>::value;
	
	// set mode
	DMA_CCR(dma, channel) = (DMA_CCR(dma, channel) & ~0x4012) | int(mode);
	return *this;
}
