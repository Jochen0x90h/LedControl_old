#pragma once

#include "generated/exp16u5.h"
#include "generated/cos8u10.h"


ParameterInfo PlasmaParameterInfos[] = {
	PARAMETER("Speed", 10, 31, 1, 20),
	PARAMETER("Period", 16, 26, 1, 20),
	PARAMETER("Saturation", 0, 255, 4, 0),
	PARAMETER("Hue", 0, 255, 4 | WRAP, 255)
};

// calculate simple sine approximation in fixed<16, 16> format
int32_t calcSin(int32_t x) {
	int32_t x1 = x & 32767;
	int32_t y = x1 * (32768 - x1) >> 12;
	if (x & 32768)
		y = -y;
	return y;
}


class Plasma : public Effect {
public:

	Plasma(int ledCount) {
	}
	
	~Plasma() override {
	}

	void run(int ledCount, uint8_t brightness, uint8_t * parameters) override {
		uint8_t speed = parameters[0];
		uint8_t period = parameters[1];
		uint8_t saturation = parameters[2];
		uint16_t hue = parameters[3] << 3;

		int32_t offset1 = this->time >> 10;
		int32_t scale1 = exp16u5(period + 1);

		int32_t offset2 = this->time >> 6;
		int32_t scale2 = exp16u5(period + 1);

		for (int16_t i = 0; i < ledCount; ++i) {
			// sine 1 changes offset over time (moving)
			uint8_t y1 = cos8u10((uint16_t)(((i << 4) + offset1) * scale1 >> 12) & 0x3ff);
			
			// sine 2 changes spatial frequency over time (wobbling)
			int32_t s = (calcSin(offset2) + 100000) * scale2 >> 8; // fixed<16, 16>
			uint8_t y2 = cos8u10((uint16_t)((i - (ledCount >> 1)) * s >> 15) & 0x3ff);
			
			uint8_t r = (y1 + y2) >> 1;
			RGB rgb = hsv2rgb(HSV(hue, saturation, r));
			sendColor(rgb);
		}
		this->time += exp16u5(speed);
	}
	
	int32_t time;
};
