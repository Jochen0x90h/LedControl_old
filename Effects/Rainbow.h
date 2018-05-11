#pragma once

#include "hsv2rgb.h"
#include "generated/exp16_5.h"


ParameterInfo RainbowParameterInfos[] = {
	PARAMETER("Brightness", 25, 255, 4, 255),
	PARAMETER("Speed", 0, 31, 1, 20),
	PARAMETER("Period", 0, 31, 1, 20),
	PARAMETER("Saturation", 0, 255, 4, 255)
};

class Rainbow : public Effect {
public:
	uint32_t startHue;

	Rainbow(int ledCount) : startHue(0) {
	}
	
	~Rainbow() override {
	}

	void run(int ledCount, uint8_t* parameters) override {
		uint8_t value = parameters[0];
		uint8_t speed = parameters[1];
		uint8_t period = parameters[2];
		uint8_t saturation = parameters[3];

		uint32_t hue = this->startHue >> 2;
		for (int i = 0; i < ledCount; ++i) {
			HSV hsv(hue >> 8, saturation, value);
			RGB rgb = hsv2rgb(hsv);
			sendColor(rgb);		
			
			hue += exp16_5(period);
		}
		this->startHue += exp16_5(speed);
	}
};
