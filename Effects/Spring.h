#pragma once

#include "noise.h"
#include "generated/spring.h"
#include "generated/exp16u8.h"
#include "generated/cos8u10.h"


const ParameterInfo SpringParameterInfos[] = {
	PARAMETER("Speed", 160, 255, 3, 200),
	PARAMETER("Period", 160, 250, 3, 200),
};

class Spring : public Effect {
public:

	uint32_t start;

	Spring(int ledCount) : start() {
	}
	
	~Spring() override {
	}
	
	void run(int ledCount, uint8_t brightness, uint8_t * parameters) override {
		uint8_t speed = parameters[0];
		uint8_t period = parameters[1];

		uint16_t exp = exp16u8(period);
		
		uint32_t cosOffset = this->start >> 6;
		uint32_t cosStep = exp;
		
		uint32_t noiseOffset = -cosOffset;
		uint32_t noiseStep = exp;
		
		for (int i = 0; i < ledCount; ++i) {
			int16_t s = (cos8u10((cosOffset >> 8) & 0x3ff) - 0x00) * (noise8s(noiseOffset >> 8));
			
			RGB color = spring((s >> 4) & 0x3ff);
			
			sendColor(scale8u(color, brightness));
			
			cosOffset += cosStep;
			noiseOffset += noiseStep;
		}
		this->start += exp16u8(speed);
	}
};
