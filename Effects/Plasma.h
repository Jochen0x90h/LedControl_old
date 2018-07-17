#pragma once

#include "generated/exp16u5.h"
#include "generated/cos8u10.h"
#include "Effect.h"


// calculate simple sine approximation in fixed<16, 16> format
int32_t calcSin(int32_t x) {
	int32_t x1 = x & 32767;
	int32_t y = x1 * (32768 - x1) >> 12;
	if (x & 32768)
		y = -y;
	return y;
}

struct Plasma {
	int32_t time;
};

void PlasmaInit(struct Plasma * effect, int ledCount) {
	effect->time = 0;
}

ParameterInfo FLASH PlasmaParameterInfos[] = {
	PARAMETER("Speed", 10, 31, 1, 20),
	PARAMETER("Period", 16, 26, 1, 20),
	PARAMETER("Saturation", 0, 255, 4, 0),
	PARAMETER("Hue", 0, 255, 4 | WRAP, 255)
};

void PlasmaRun(struct Plasma * effect, int ledCount, uint8_t brightness, uint8_t * parameters) {
	uint8_t speed = parameters[0];
	uint8_t period = parameters[1];
	uint8_t saturation = parameters[2];
	uint16_t hue = parameters[3] << 3;

	int32_t add1 = (int32_t)(exp16u5(period + 1)) << 8;
	int32_t offset1 = (effect->time >> 10) * add1 >> 3;

	int32_t offset2 = effect->time >> 6;
	int32_t scale2 = exp16u5(period + 1);

	for (int16_t i = 0; i < ledCount; ++i) {
		// sine 1 changes offset over time (moving)
		uint8_t y1 = cos8u10((offset1 >> 16) & 0x3ff);
		
		// sine 2 changes spatial frequency over time (wobbling)
		int32_t s = (calcSin(offset2) + 100000) * scale2 >> 8; // fixed<16, 16>
		uint8_t y2 = cos8u10((uint16_t)((i - (ledCount >> 1)) * s >> 15) & 0x3ff);
		
		uint8_t r = (y1 + y2) >> 1;
		RGB color = hsv2rgb(makeHSV(hue, saturation, r));
		sendRGB(scale8RGB(color, brightness));
		
		offset1 += add1;
	}
	effect->time += exp16u5(speed);
}
