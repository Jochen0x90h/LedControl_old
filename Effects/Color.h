#pragma once

#include "hsv2rgb.h"
#include "Effect.h"
#include "generated/hue.h"


struct Color {

};

void ColorInit(struct Color * effect, int ledCount) {
}

ParameterInfo FLASH ColorParameterInfos[] = {
	PARAMETER("Saturation", 0, 255, 8, 255),
	PARAMETER("Hue", 0, 255, 8 | WRAP, 255)
};

void ColorRun(struct Color * effect, int ledCount, uint8_t brightness, uint8_t * parameters) {
	uint8_t saturation = parameters[0];
	uint16_t hue = parameters[1] << 3;

	RGB rgb = hsv2rgb(makeHSV(hue, saturation, brightness));
	for (int i = 0; i < ledCount; ++i) {
		sendRGB(rgb);
	}
}
