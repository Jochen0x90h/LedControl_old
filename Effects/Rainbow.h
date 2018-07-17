#pragma once

#include "hsv2rgb.h"
#include "generated/exp16u5.h"
#include "Effect.h"


struct Rainbow {
	uint32_t start;
};

void RainbowInit(struct Rainbow * effect, int ledCount) {
	effect->start = 0;
}

ParameterInfo FLASH RainbowParameterInfos[] = {
	PARAMETER("Speed", 10, 31, 1, 20),
	PARAMETER("Period", 0, 31, 1, 20),
	PARAMETER("Saturation", 0, 255, 4, 255)
};

void RainbowRun(struct Rainbow * effect, int ledCount, uint8_t brightness, uint8_t * parameters) {
	uint8_t speed = parameters[0];
	uint8_t period = parameters[1];
	uint8_t saturation = parameters[2];

	uint32_t hue = effect->start;
	uint16_t step = exp16u5(period);
	for (int i = 0; i < ledCount; ++i) {
		HSV hsv = makeHSV((hue >> 8) & 0x7ff, saturation, brightness);
		RGB rgb = hsv2rgb(hsv);
		sendRGB(rgb);
		
		hue += step;
	}
	effect->start += exp16u5(speed);
}
