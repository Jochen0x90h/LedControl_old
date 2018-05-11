#pragma once

#include "generated/exp16_5.h"


ParameterInfo MovingLightParameterInfos[] = {
	PARAMETER("Brightness", 25, 255, 4, 255),
	PARAMETER("Speed", 0, 31, 1, 10),
	PARAMETER("Length", 2, 22, 1, 10),
	PARAMETER("Duty Cycle", 0, 255, 4, 128),
	PARAMETER("Hue", 0, 255, 4, 255),
	PARAMETER("Saturation", 0, 255, 4, 255)
};

class MovingLight : public Effect {
public:

	uint32_t start;

	MovingLight(int ledCount) : start(0) {
	}
	
	~MovingLight() override {
	}

	void run(int ledCount, uint8_t* parameters) override {
		uint8_t brightness = parameters[0];
		uint8_t speed = parameters[1];
		uint8_t length = parameters[2];
		uint8_t dutyCycle = parameters[3];
		uint16_t hue = parameters[4] << 3;
		uint8_t saturation = parameters[5];

		uint16_t count = exp16_5(length);
		uint16_t onCount = scale16_8(count - 2, dutyCycle) + 1;
		uint16_t j = this->start >> 8;
		uint8_t frac = (uint8_t)this->start;
		if (j >= count) {
			this->start -= count << 8;
			j -= count;
		}
		
		RGB rgb1 = hsv2rgb(HSV(hue, saturation, brightness));
		RGB rgb2 = hsv2rgb(HSV(hue, saturation, scale8(brightness, 255 - frac)));
		RGB rgb3(0, 0, 0);
		RGB rgb4 = hsv2rgb(HSV(hue, saturation, scale8(brightness, frac)));
		
		for (int i = 0; i < ledCount; ++i) {
			++j;
			if (j <= onCount) {
				if (j < onCount) {
					sendColor(rgb1);
				} else {
					sendColor(rgb2);
				}
			} else {
				if (j < count) {
					sendColor(rgb3);
				} else {
					j = 0;
					sendColor(rgb4);
				}
			}
		}
		
		this->start += scale16(count << 7, exp16_5(speed));
	}
};
