#pragma once

#include "noise.h"
#include "generated/autumn.h"


const ParameterInfo AutumnParameterInfos[] = {
	PARAMETER("Speed", 160, 255, 3, 200),
	PARAMETER("Period", 180, 220, 2, 200),
};

class Autumn : public Effect {
public:

	uint32_t start;

	Autumn(int ledCount) : start() {
	}
	
	~Autumn() override {
	}
	
	void run(int ledCount, uint8_t brightness, uint8_t * parameters) override {
		uint8_t speed = parameters[0];
		uint8_t period = parameters[1];
		
		uint16_t exp = exp16u8(period);

		uint32_t noiseOffset1 = this->start >> 6;
		uint32_t noiseStep1 = exp;
		
		uint32_t noiseOffset2 = -noiseOffset1;
		uint32_t noiseStep2 = exp;


		uint32_t noiseOffset3 = this->start >> 10;
		uint32_t noiseStep3 = exp >> 3;
		
		uint32_t noiseOffset4 = -noiseOffset3;
		uint32_t noiseStep4 = exp >> 3;

		for (int i = 0; i < ledCount; ++i) {
			
			// leaves
			uint8_t l = (noise8s(noiseOffset1 >> 8) * noise8s(noiseOffset2 >> 8) >> 7) + 80;

			// tree trunks
			uint8_t t = (noise8s(noiseOffset3 >> 8) * noise8s(noiseOffset4 >> 8) >> 7) + 170;//128;
			
			RGB leaves = autumn(l << 2);
			
			uint8_t s = scale8u(t, brightness);
			sendColor(scale8u(leaves, s));
			
			noiseOffset1 += noiseStep1;
			noiseOffset2 += noiseStep2;
			noiseOffset3 += noiseStep3;
			noiseOffset4 += noiseStep4;
		}
		this->start += exp16u8(speed);
	}
};
