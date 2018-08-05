#pragma once

#include "noise.h"
#include "generated/autumn.h"
#include "Effect.h"


struct Autumn {
	uint32_t start;
};

void AutumnInit(struct Autumn * effect, int ledCount) {
	effect->start = 0;
}

ParameterInfo FLASH AutumnParameterInfos[] = {
	PARAMETER("Speed", 160, 255, 3, 200),
	PARAMETER("Period", 180, 220, 2, 200),
};

void AutumnRun(struct Autumn * effect, int ledCount, uint8_t brightness, uint8_t * parameters) {
	uint8_t speed = parameters[0];
	uint8_t period = parameters[1];

	uint16_t exp = exp16u8(period);

	uint32_t noiseOffset1 = effect->start >> 6;
	uint32_t noiseStep1 = exp;

	uint32_t noiseOffset2 = -noiseOffset1;
	uint32_t noiseStep2 = exp;


	uint32_t noiseOffset3 = effect->start >> 10;
	uint32_t noiseStep3 = exp >> 3;

	uint32_t noiseOffset4 = -noiseOffset3;
	uint32_t noiseStep4 = exp >> 3;

	for (int i = 0; i < ledCount; ++i) {
		// leaves
		uint16_t l = (noise8s(noiseOffset1 >> 8) * noise8s(noiseOffset2 >> 8) >> 5) + 80*4;
		RGB leaves = autumn(l & 0x3ff);

		// tree trunks (currently too slow on atmega328)
		//uint8_t t = (noise8s(noiseOffset3 >> 8) * noise8s(noiseOffset4 >> 8) >> 7) + 170;//128;
		
		//uint8_t s = scale8u(t, brightness);
		//sendRGB(scale8RGB(leaves, s));
		sendRGB(scale8RGB(leaves, brightness));

		noiseOffset1 += noiseStep1;
		noiseOffset2 += noiseStep2;
		noiseOffset3 += noiseStep3;
		noiseOffset4 += noiseStep4;
	}
	effect->start += exp16u8(speed);
}
