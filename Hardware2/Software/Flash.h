#pragma once

#include <libopencm3/stm32/flash.h>

/**
 * Stores data in flash memory by filling up a set of flash pages with address/value pairs and copying 
 * the latest value for each address to another set of pages if the current set is full. This increases
 * the number of possible write cycles from typically 10k for flash memory by the ratio between free space
 * and number of stored values. For example if we want to store 256 values but use space for 4096 values then
 * the number of possible write cycles increases by a factor of 16 (160k). 
 * Reference Manual, Page 77: https://www.st.com/content/ccc/resource/technical/document/reference_manual/group0/b0/ac/3e/8f/6d/21/47/af/DM00151940/files/DM00151940.pdf/jcr:content/translations/en.DM00151940.pdf
 * EEPROM Emulation: https://www.st.com/content/ccc/resource/technical/document/application_note/group0/b2/94/a6/62/18/c0/4f/e6/DM00311483/files/DM00311483.pdf/jcr:content/translations/en.DM00311483.pdf
 * http://libopencm3.org/docs/latest/stm32l4/html/group__flash__file.html
 * https://www.st.com/content/ccc/resource/training/technical/product_training/91/e3/aa/26/e6/69/4f/de/STM32L4_Memory_Flash.pdf/files/STM32L4_Memory_Flash.pdf/jcr:content/translations/en.STM32L4_Memory_Flash.pdf
 */
class Flash {
public:

	Flash(int pageBegin, int pageCount) {}
	
	void setup() {
	}
	
	void write8(uint16_t address, uint8_t value) {
	}
	
	uint8_t read8(int address, uint8_t defaultValue = 0xff) {
		return defaultValue;
	}
};
