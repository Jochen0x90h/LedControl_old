#pragma once

#include "noise.h"
#include "generated/summer.h"
#include "generated/cos8u10.h"


const ParameterInfo SummerParameterInfos[] = {
	PARAMETER("Speed", 20, 31, 1, 27),
	PARAMETER("Period", 2, 6, 1, 4),
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

		uint16_t exp = exp16u5(period);
		uint16_t noiseOffset1 = this->start >> 16;
		uint16_t noiseStep1 = exp >> 1;
		uint16_t noiseOffset2 = -noiseOffset1;
		uint16_t noiseStep2 = exp;
		uint16_t sinOffset = this->start >> 12;
		uint16_t sinStep = exp << 4;
		for (int i = 0; i < ledCount; ++i) {
			// noise as "sun rays"
			int16_t n = noise8s(noiseOffset1) * noise8s(noiseOffset2) >> 4;
			
			// sine as envelope
			uint8_t s = cos8u10(sinOffset & 0x3ff);
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
		this->start += exp16u5(speed);
	}
};
