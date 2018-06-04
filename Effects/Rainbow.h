#pragma once

#include "hsv2rgb.h"
#include "generated/exp16u5.h"


ParameterInfo RainbowParameterInfos[] = {
	PARAMETER("Speed", 10, 31, 1, 20),
	PARAMETER("Period", 0, 31, 1, 20),
	PARAMETER("Saturation", 0, 255, 4, 255)
};

class Rainbow : public Effect {
public:
	uint32_t start;

	Rainbow(int ledCount) : start(0) {
	}
	
	~Rainbow() override {
	}

	void run(int ledCount, uint8_t brightness, uint8_t* parameters) override {
		uint8_t speed = parameters[0];
		uint8_t period = parameters[1];
		uint8_t saturation = parameters[2];

		uint32_t hue = this->start;
		uint16_t step = exp16u5(period);
		for (int i = 0; i < ledCount; ++i) {
			HSV hsv((hue >> 8) & 0x7ff, saturation, brightness);
			RGB rgb = hsv2rgb(hsv);
			sendColor(rgb);		
			
			hue += step;
		}
		this->start += exp16u5(speed);
	}
};
