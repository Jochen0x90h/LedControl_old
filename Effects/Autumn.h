#pragma once

#include "noise.h"
#include "generated/autumn.h"


const ParameterInfo AutumnParameterInfos[] = {
	PARAMETER("Speed", 20, 31, 1, 27),
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
		
		
		uint16_t lo = this->start >> 14;
		uint16_t to = this->start >> 18;
		for (int i = 0; i < ledCount; ++i) {
			
			uint8_t l = (noise8s((i << 5) + lo) * noise8s((i << 5) - lo) >> 7) + 80;
			//uint8_t t = (noise8s((i << 2) + to) * noise8s((i << 2) - to) >> 7) > 10 ? brightness >> 1 : brightness;
			uint8_t t = (noise8s((i << 2) + to) * noise8s((i << 2) - to) >> 7) + 128;
			//l += t;
			
			RGB leaves = autumn(l << 2);
			
			uint8_t s = scale8(t, brightness);
			sendColor(RGB(
					scale8(leaves.r, s),
					scale8(leaves.g, s),
					scale8(leaves.b, s)));
		}
		this->start += exp16u5(speed);
	}
};
