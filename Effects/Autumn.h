#pragma once

#include "noise.h"
#include "generated/autumnGradient.h"


const ParameterInfo AutumnParameterInfos[] = {
	PARAMETER("Brightness", 25, 255, 4, 255),
};

class Autumn : public Effect {
public:

	uint32_t start;

	Autumn(int ledCount) : start() {
	}
	
	~Autumn() override {
	}
	
	void run(int ledCount, uint8_t * parameters) override {
		uint8_t brightness = parameters[0];
		
		
		uint16_t lo = this->start;
		uint16_t to = this->start >> 4;
		for (int i = 0; i < ledCount; ++i) {
			
			uint8_t l = (noise8s((i << 5) + lo) * noise8s((i << 5) - lo) >> 7) + 80;
			//uint8_t t = (noise8s((i << 2) + to) * noise8s((i << 2) - to) >> 7) > 10 ? brightness >> 1 : brightness;
			uint8_t t = (noise8s((i << 2) + to) * noise8s((i << 2) - to) >> 7) + 80;
			//l += t;
			
			RGB leaves = autumnGradient(l >> 6, l << 2);
			
			
			sendColor(RGB(
					scale8(leaves.r, t),
					scale8(leaves.g, t),
					scale8(leaves.b, t)));
			//sendColor(RGB(x, x, x));
		}
		this->start += 1;
		//movingLight.start += exp16(speed);
	}
};
