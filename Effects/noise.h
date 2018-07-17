#pragma once

#include "generated/permute8.h"

// inspired by FastLED noise functions: http://fastled.io/docs/3.1/noise_8cpp_source.html

// helper function: calculate bumps of varying height in the range -128..127
inline int8_t bump8s(uint8_t i, uint8_t x) {
	int8_t hash = permute8(i);
	int8_t y = (uint8_t)(x * x >> 7) - (uint8_t)(x + x);
	
	int8_t u, v;
	if (hash & 8) {
		u = y;
		v = y;
	} else {
		if (hash & 4) {
			u = 1;
			v = y;
		} else {
			u = y;
			v = 1;
		}
	}

	if (hash & 1)
		u = ~u;
	if (hash & 2)
		v = ~v;

	return avg8s(u, v);
}

// generate noise in the range -128..127 for x in 8.8 fixed point notation
inline int8_t noise8s(uint16_t x) {
	// integer part of x
	uint8_t i = x >> 8;
	
	// fractional part of x
	uint8_t f = (uint8_t)x;

	// bump for position i and negative bump for next position
	int8_t g1 = bump8s(i, f);
	int8_t g2 = ~bump8s(i + 1, f);
	
	// interpolate between the bumps
	return lerp8s(g1, g2, f);
}
