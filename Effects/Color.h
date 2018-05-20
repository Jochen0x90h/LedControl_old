#pragma once

#include "hsv2rgb.h"


ParameterInfo ColorParameterInfos[] = {
	PARAMETER("Saturation", 0, 255, 4, 255),
	PARAMETER("Hue", 0, 255, 4 | WRAP, 255)
};

class Color : public Effect {
public:

	Color(int ledCount) {
	}
	
	~Color() override {
	}

	void run(int ledCount, uint8_t brightness, uint8_t * parameters) override {
		uint8_t saturation = parameters[0];
		uint16_t hue = parameters[1] << 3;

		RGB rgb = hsv2rgb(HSV(hue, saturation, brightness));
		for (int i = 0; i < ledCount; ++i) {
			sendColor(rgb);
		}
	}
};
