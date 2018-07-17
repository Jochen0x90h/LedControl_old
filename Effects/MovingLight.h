#pragma once

#include "generated/exp16u5.h"
#include "Effect.h"


struct MovingLight {
	uint32_t start;
};

void MovingLightInit(struct MovingLight * effect, int ledCount) {
	effect->start = 0;
}

ParameterInfo FLASH MovingLightParameterInfos[] = {
	PARAMETER("Speed", 6, 28, 1, 10),
	PARAMETER("Length", 2, 22, 1, 10),
	PARAMETER("Duty Cycle", 0, 255, 4, 128),
	PARAMETER("Hue", 0, 255, 4, 255),
	PARAMETER("Saturation", 0, 255, 4, 0)
};

void MovingLightRun(struct MovingLight * effect, int ledCount, uint8_t brightness, uint8_t * parameters) {
	uint8_t speed = parameters[0];
	uint8_t length = parameters[1];
	uint8_t dutyCycle = parameters[2];
	uint16_t hue = parameters[3] << 3;
	uint8_t saturation = parameters[4];

	uint16_t count = exp16u5(length);
	uint16_t onCount = scale16u8u(count - 2, dutyCycle) + 1;
	uint16_t j = effect->start >> 8;
	uint8_t frac = (uint8_t)effect->start;
	if (j >= count) {
		effect->start -= count << 8;
		j -= count;
	}

	// colors for on and off state and transitions
	RGB onColor = hsv2rgb(makeHSV(hue, saturation, brightness));
	RGB on2offColor = hsv2rgb(makeHSV(hue, saturation, scale8u(brightness, 255 - frac)));
	RGB offColor = {0, 0, 0};
	RGB off2onColor = hsv2rgb(makeHSV(hue, saturation, scale8u(brightness, frac)));

	for (int i = 0; i < ledCount; ++i) {
		++j;
		if (j <= onCount) {
			// on
			if (j < onCount) {
				sendRGB(onColor);
			} else {
				sendRGB(on2offColor);
			}
		} else {
			// off
			if (j < count) {
				sendRGB(offColor);
			} else {
				j = 0;
				sendRGB(off2onColor);
			}
		}
	}

	effect->start += scale16u(count << 7, exp16u5(speed));
}
