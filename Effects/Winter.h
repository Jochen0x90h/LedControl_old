#pragma once

#include "noise.h"
#include "generated/winter.h"


const ParameterInfo WinterParameterInfos[] = {
	PARAMETER("Speed", 20, 31, 1, 27),
};

class Winter : public Effect {
public:

	uint32_t start;

	Winter(int ledCount) : start() {
	}
	
	~Winter() override {
	}
	
	void run(int ledCount, uint8_t brightness, uint8_t * parameters) override {
		uint8_t speed = parameters[0];
		
		
		uint16_t lo = this->start >> 14;
		//uint16_t to = this->start >> 18;
		for (int i = 0; i < ledCount; ++i) {
			
			//int8_t stars1 = (noise8s((i << 5) + lo) * noise8s((i << 5) - lo) >> 7);
			//uint8_t stars2 = stars1 > 20 ? stars1 - 20 : 0;
			//uint8_t stars = stars2 < 16 ? stars2 << 4 : 255;

			int16_t n = noise8s((i << 5) + lo) * noise8s((i << 4) - lo) >> 4;
			RGB color = winter(n & 0x3ff);
/*
			//uint8_t t = (noise8s((i << 2) + to) * noise8s((i << 2) - to) >> 7) > 10 ? brightness >> 1 : brightness;
			uint8_t t = (noise8s((i << 2) + to) * noise8s((i << 2) - to) >> 7) + 128;
			//l += t;
			
			RGB leaves = autumn(l << 2);
*/
			//uint8_t s = background;//scale8(t, brightness);
			//sendColor(background);//RGB(s, s, s));
			//sendColor(RGB(stars, stars, (stars >> 1) + 128));
			
			sendColor(scale8u(color, brightness));//RGB(
					//scale8(color.r, brightness),
					//scale8(color.g, brightness),
					//scale8(color.b, brightness)));
		}
		this->start += exp16u5(speed);
	}
};
