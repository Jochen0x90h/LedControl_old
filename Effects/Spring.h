#pragma once

#include "noise.h"
#include "generated/spring.h"
#include "generated/exp16u8.h"
#include "generated/cos8u10.h"
#include "Effect.h"


struct Spring {
	uint32_t start;
};

void SpringInit(struct Spring * effect, int ledCount) {
	effect->start = 0;
}

ParameterInfo FLASH SpringParameterInfos[] = {
	PARAMETER("Speed", 160, 255, 3, 200),
	PARAMETER("Period", 160, 250, 3, 200),
};

void SpringRun(struct Spring * effect, int ledCount, uint8_t brightness, uint8_t * parameters) {
	uint8_t speed = parameters[0];
	uint8_t period = parameters[1];

	uint16_t exp = exp16u8(period);

	uint32_t cosOffset = effect->start >> 6;
	uint32_t cosStep = exp;

	uint32_t noiseOffset = -cosOffset;
	uint32_t noiseStep = exp;

	for (int i = 0; i < ledCount; ++i) {
		int16_t s = (cos8u10((cosOffset >> 8) & 0x3ff) - 0x00) * (noise8s(noiseOffset >> 8));
		
		RGB color = spring((s >> 4) & 0x3ff);
		
		sendRGB(scale8RGB(color, brightness));
		
		cosOffset += cosStep;
		noiseOffset += noiseStep;
	}
	effect->start += exp16u8(speed);
}
