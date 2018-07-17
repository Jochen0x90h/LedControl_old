#pragma once

#include "noise.h"
#include "generated/winter.h"
#include "Effect.h"


struct Winter {
	uint32_t start;
};

void WinterInit(struct Winter * effect, int ledCount) {
	effect->start = 0;
}

ParameterInfo FLASH WinterParameterInfos[] = {
	PARAMETER("Speed", 160, 255, 3, 200),
	PARAMETER("Period", 180, 220, 2, 200),
};

void WinterRun(struct Winter * effect, int ledCount, uint8_t brightness, uint8_t * parameters) {
	uint8_t speed = parameters[0];
	uint8_t period = parameters[1];

	uint16_t exp = exp16u8(period);

	uint32_t noiseOffset1 = effect->start >> 6;
	uint32_t noiseStep1 = exp;

	uint32_t noiseOffset2 = -noiseOffset1;
	uint32_t noiseStep2 = exp >> 1;

	for (int i = 0; i < ledCount; ++i) {
		int16_t n = noise8s(noiseOffset1 >> 8) * noise8s(noiseOffset2 >> 8) >> 4;
		RGB color = winter(n & 0x3ff);
		
		sendRGB(scale8RGB(color, brightness));

		noiseOffset1 += noiseStep1;
		noiseOffset2 += noiseStep2;
	}
	effect->start += exp16u8(speed);
}
