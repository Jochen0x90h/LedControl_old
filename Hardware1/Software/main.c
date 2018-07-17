#define F_CPU 8000000UL
#define LED_COUNT 300

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h> // http://www.nongnu.org/avr-libc/user-manual/pgmspace.html
#include <avr/eeprom.h> // http://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html
#include <util/delay.h>
#include <stdint.h>
#include "types.h"
#include "util.h"


#define DD_SS PB2
#define DD_MOSI PB3
#define DD_MISO PB4
#define DD_SCK PB5

// enable SPI, Set as Master, Prescaler: Fosc/32, Enable Interrupts
//const uint8_t SPI_CONFIG = (1 << SPE) | (1 << MSTR) | (1 << SPI2X) | (1 << SPR1) | (1 << CPHA) | (1 << SPIE);
	
// enable SPI, Set as Master, Prescaler: Fosc/16, Enable Interrupts
const uint8_t SPI_CONFIG = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << CPHA) | (1 << SPIE);




// encoders

uint8_t lastPinC;
uint8_t lastPinD;

typedef struct {
	uint8_t pins;
	int8_t count;
} Encoder;
Encoder encoders[4] = {};

void updateEncoder(Encoder * encoder, uint8_t pins) {
	uint8_t lastPins = encoder->pins;
	encoder->pins = pins;
	if (pins == 1 && lastPins == 0) {
		++encoder->count;
	} else if (pins == 0 && lastPins == 1) {
		--encoder->count;
	}
}

void pollEncoders() {
	uint8_t pinC = PINC;
	uint8_t pinD = PIND;
	
	// check if any encoder pin has changed
	if ((pinC ^ lastPinC) & 0x3f || (pinD ^ lastPinD) & 0x0c) {
		lastPinC = pinC;
		lastPinD = pinD;

		// top encoder		
		uint8_t pins0 = pinC & 3;
		updateEncoder(&encoders[0], pins0);
	
		// bottom left encoder
		uint8_t pins1 = (pinC >> 2) & 3;
		updateEncoder(&encoders[1], pins1);
	
		// bottom middle encoder
		uint8_t pins2 = (pinC >> 4) & 3;
		updateEncoder(&encoders[2], pins2);
	
		// bottom right encoder
		uint8_t pins3 = (pinD >> 2) & 3;
		updateEncoder(&encoders[3], pins3);
	}
}


// spi

struct {
	volatile uint8_t step;
	uint8_t buf1;
	uint8_t buf2;
} spi;

ISR(SPI_STC_vect) {
	if (spi.step < 3) {
 		// send red and blue
 		SPDR = spi.buf1;
 		spi.buf1 = spi.buf2;
		++spi.step; 	
 	} else {
 		spi.step = 0;
 	}
}

void sendColor(uint8_t red, uint8_t green, uint8_t blue) {
	// wait until last transmission is complete
	while (spi.step != 0);
    
	// set red and blue to spi buffer to send later
	spi.buf1 = ~red;
	spi.buf2 = ~blue;

	// send green
	SPDR = ~green;
	spi.step = 1;

	// poll encoders here so we don't miss a state if we turn them fast
	pollEncoders();
}

inline void sendRGB(RGB color) {
	sendColor(color.red, color.green, color.blue);
}


// effects (use sendColor() and sendRGB())
#include "Color.h"
#include "Rainbow.h"
#include "MovingLight.h"
#include "Spring.h"
#include "Summer.h"
#include "Autumn.h"
#include "Winter.h"
#include "Plasma.h"
EffectInfo FLASH effectInfos[] = {
	EFFECT(Color),
	EFFECT(Rainbow),
	EFFECT(Spring),
	EFFECT(Summer),
	EFFECT(Autumn),
	EFFECT(Winter),
	EFFECT(Plasma),
	EFFECT(MovingLight),
};

// brightness parameter
ParameterInfo FLASH brightnessInfo = PARAMETER("Brightness", 25, 255, 4, 255);

// parameter value to display instead of mode
uint8_t ledIndex;
uint8_t ledTimeout = 0;

void updateParameter(uint8_t * value, ParameterInfo FLASH * parameterInfo, int8_t count, int eeOffset) {
	if (count == 0)
		return;
	
	// read parameter info from flash
	uint8_t minValue = parameterInfo->minValue;
	uint8_t maxValue = parameterInfo->maxValue;
	uint8_t step = parameterInfo->step;
	
	// modify parameter value
	int16_t newValue = *value + count * (int8_t)(step & ~WRAP);
	if (newValue < minValue) {
		if (step & WRAP)
			newValue += maxValue - minValue;
		else
			newValue = minValue;
	} else if (newValue > maxValue) {
		if (step & WRAP)
			newValue -= maxValue - minValue;
		else
			newValue = maxValue;
	}
	*value = (uint8_t)newValue;
	
	// write value to eeprom
	eeprom_write_byte((uint8_t *)eeOffset, *value);
	
	// display value on local led's for one second
	ledIndex = (14 * (*value - minValue)) / (maxValue - minValue + 1);
	ledTimeout = 100;	
}

int main (void) {
	// effects
	uint8_t const effectCount = sizeof(effectInfos) / sizeof(EffectInfo);
	uint8_t effectIndex = eeprom_read_byte(0);
	
	// parameters
	uint8_t const maxParameterCount = 5;
	uint8_t brightness = eeprom_read_byte((uint8_t const *)1);
	uint8_t parameters[maxParameterCount];
	
	// check if eeprom was initialized
	if (effectIndex >= effectCount) {
		effectIndex = 0;
		eeprom_write_byte(0, effectIndex);
		
		// set all parameters to default values in eeprom
		for (uint8_t effectIndex = 0; effectIndex < effectCount; ++effectIndex) {
			EffectInfo FLASH * effectInfo = &effectInfos[effectIndex];
			uint8_t parameterCount = effectInfo->parameterCount;
			for (uint8_t parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex) {
				parameters[parameterIndex] = effectInfo->parameterInfos[parameterIndex].initValue;
			}
			eeprom_write_block(parameters, (uint8_t *)2 + effectIndex * maxParameterCount, parameterCount);	
		}
	}
	
	// current effect
	EffectInfo FLASH * effectInfo = NULL;
	uint8_t effectData[32];
	uint8_t parameterCount;
	size_t eeOffset = 0;

	// loop
	while (1) {	
		// check if effect encoder was turned
		if (encoders[0].count != 0) {
			effectIndex += encoders[0].count;
			encoders[0].count = 0;
			if (effectIndex < 0)
				effectIndex = effectCount - 1;
			if (effectIndex >= effectCount)
				effectIndex = 0;		

			// write effect index to eeprom
			eeprom_write_byte(0, effectIndex);
		}			

		// display an effect parameter on the local led's during timeout, otherwise display effect index
		if (ledTimeout > 0)
			--ledTimeout;
		else
			ledIndex = 13 - effectIndex;
				
		// switch on the local led at ledIndex
		uint8_t portb = PORTB & ~0x83;
		uint8_t ddrb = DDRB & ~0x83;
		uint8_t portd = PORTD & ~0xf0;
		uint8_t ddrd = DDRD & ~0xf0;
		if (ledIndex <= 1) {
			// 0 - 1
			uint8_t bit = 1 << ledIndex;
			ddrb |= bit;
		} else if (ledIndex == 2) {
			ddrb |= (1 << PB7);
		} else if (ledIndex <= 6) {
			// 3 - 6
			int8_t bit = 1 << (4 + ledIndex - 3);
			ddrd |= bit;
		} else if (ledIndex <= 8) {
			// 7 - 8
			uint8_t bit = 1 << (ledIndex - 7);
			portb |= bit;
			ddrb |= bit;
		} else if (ledIndex == 9) {
			portb |= (1 << PB7);
			ddrb |= (1 << PB7);
		} else if (ledIndex <= 13) {
			// 10 - 13
			uint8_t bit = 1 << (4 + ledIndex - 10);
			portd |= bit;
			ddrd |= bit;
		}
		PORTB = portb;
		PORTD = portd;
		DDRB = ddrb;
		DDRD = ddrd;

		// check if effect has changed
		EffectInfo FLASH * nextEffectInfo = &effectInfos[effectIndex];
		if (effectInfo != nextEffectInfo) {
			effectInfo = nextEffectInfo;
			parameterCount = effectInfo->parameterCount;

			// read all effect parameters from eeprom
			eeOffset = 2 + effectIndex * maxParameterCount;
			eeprom_read_block(parameters, (uint8_t const *)eeOffset, parameterCount);

			// init new effect
			effectInfo->init(effectData, LED_COUNT);
		}

		// check if parameter has changed
		if (1) {
			updateParameter(&brightness, &brightnessInfo, encoders[1].count, 1);	
			updateParameter(&parameters[0], &effectInfo->parameterInfos[0], encoders[2].count, eeOffset + 0);	
			updateParameter(&parameters[1], &effectInfo->parameterInfos[1], encoders[3].count, eeOffset + 1);	
		} else {
			for (uint8_t i = 2; i < parameterCount; ++i) { 
				updateParameter(&parameters[i], &effectInfo->parameterInfos[i], encoders[i - 1].count, eeOffset + i);
			}
		}

		// run effect
		effectInfo->run(effectData, LED_COUNT, brightness, parameters);

		// wait until last transmission is complete
		while (spi.step != 0);
		pollEncoders();
		
		// wait at least 50 us
		_delay_us(50);
		pollEncoders();
	}
	return 0;
}
