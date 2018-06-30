#pragma once

#include "noise.h"
#include "generated/cos8u10.h"
#include "generated/exp16u8.h"
#include "generated/summer.h"


const ParameterInfo SummerParameterInfos[] = {
	PARAMETER("Speed", 160, 255, 3, 200),
	PARAMETER("Period", 120, 180, 2, 150),
};

class Summer : public Effect {
public:

	uint32_t start;

	Summer(int ledCount) : start() {
	}
	
	~Summer() override {
	}
	
	void run(int ledCount, uint8_t brightness, uint8_t * parameters) override {
		uint8_t speed = parameters[0];
		uint8_t period = parameters[1];

		uint16_t exp = exp16u8(period);
		
		uint32_t noiseOffset1 = this->start >> 8;
		uint32_t noiseStep1 = exp >> 1;
		
		uint32_t noiseOffset2 = -noiseOffset1;
		uint32_t noiseStep2 = exp;
		
		uint32_t sinOffset = this->start >> 4;
		uint32_t sinStep = exp << 4;
		
		for (int i = 0; i < ledCount; ++i) {
			// noise as "sun rays"
			int16_t n = noise8s(noiseOffset1 >> 8) * noise8s(noiseOffset2 >> 8) >> 4;
			
			// sine as envelope
			uint8_t s = cos8u10((sinOffset >> 8) & 0x3ff);
			int16_t x = scale16s8u(n, s);
			
			// color lookup
			RGB color = summer(x & 0x3ff);
			
			// send
			sendColor(scale8u(color, brightness));
			
			// increment offsets
			noiseOffset1 += noiseStep1;
			noiseOffset2 += noiseStep2;
			sinOffset += sinStep;
		}
		this->start += exp16u8(speed);
	}
};
