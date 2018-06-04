#pragma once

#include "noise.h"
#include "generated/spring.h"
#include "generated/cos8u10.h"


const ParameterInfo SpringParameterInfos[] = {
	PARAMETER("Speed", 20, 31, 1, 27),
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
		
		
		uint16_t lo = this->start >> 14;
		uint16_t to = this->start >> 18;
		for (int i = 0; i < ledCount; ++i) {
			
			int16_t s = (cos8u10(((i << 5) + lo) & 0x3ff) - 0x00) * (noise8s((i << 5) - lo));
			
			RGB color = spring((s >> 4) & 0x3ff);
			
			sendColor(scale8u(color, brightness));
		}
		this->start += exp16u5(speed);
	}
};
