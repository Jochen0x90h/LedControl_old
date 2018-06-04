#pragma once

#include "generated/exp16u5.h"


ParameterInfo MovingLightParameterInfos[] = {
	PARAMETER("Speed", 6, 28, 1, 10),
	PARAMETER("Length", 2, 22, 1, 10),
	PARAMETER("Duty Cycle", 0, 255, 4, 128),
	PARAMETER("Hue", 0, 255, 4, 255),
	PARAMETER("Saturation", 0, 255, 4, 0)
};

class MovingLight : public Effect {
public:

	uint32_t start;

	MovingLight(int ledCount) : start(0) {
	}
	
	~MovingLight() override {
	}

	void run(int ledCount, uint8_t brightness, uint8_t* parameters) override {
		uint8_t speed = parameters[0];
		uint8_t length = parameters[1];
		uint8_t dutyCycle = parameters[2];
		uint16_t hue = parameters[3] << 3;
		uint8_t saturation = parameters[4];

		uint16_t count = exp16u5(length);
		uint16_t onCount = scale16u8u(count - 2, dutyCycle) + 1;
		uint16_t j = this->start >> 8;
		uint8_t frac = (uint8_t)this->start;
		if (j >= count) {
			this->start -= count << 8;
			j -= count;
		}
		
		// colors for on and off state and transitions
		RGB onColor = hsv2rgb(HSV(hue, saturation, brightness));
		RGB on2offColor = hsv2rgb(HSV(hue, saturation, scale8u(brightness, 255 - frac)));
		RGB offColor(0, 0, 0);
		RGB off2onColor = hsv2rgb(HSV(hue, saturation, scale8u(brightness, frac)));
		
		for (int i = 0; i < ledCount; ++i) {
			++j;
			if (j <= onCount) {
				// on
				if (j < onCount) {
					sendColor(onColor);
				} else {
					sendColor(on2offColor);
				}
			} else {
				// off
				if (j < count) {
					sendColor(offColor);
				} else {
					j = 0;
					sendColor(off2onColor);
				}
			}
		}
		
		this->start += scale16u(count << 7, exp16u5(speed));
	}
};
