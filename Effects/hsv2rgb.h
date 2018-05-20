#pragma once

#include "generated/hue.h"


// hsv to rgb using rainbow scheme
// see https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
inline RGB hsv2rgb(const HSV& hsv) {
	// convert hue to rgb
	RGB rgb = hue(hsv.h);
	
	// now apply saturation and value
	// rgb = (color * saturation + (1 - saturation)) * value
	// => rgb = color * saturation * value + value - saturation * value
	uint8_t sat = hsv.s;
	uint8_t val = hsv.v;
	uint8_t sat_val = scale8(sat, val);
	return RGB(
		scale8(rgb.r, sat_val) + val - sat_val,
		scale8(rgb.g, sat_val) + val - sat_val,
		scale8(rgb.b, sat_val) + val - sat_val);
}
