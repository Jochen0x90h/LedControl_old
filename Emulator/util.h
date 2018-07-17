#pragma once

#include <stdint.h>
#include "types.h"

// puts a variable into flash memory (on a microcontroller)
#define FLASH const


// from https://github.com/FastLED/FastLED/blob/master/hsv2rgb.cpp
//
// Sometimes the compiler will do clever things to reduce
// code size that result in a net slowdown, if it thinks that
// a variable is not used in a certain location.
// This macro does its best to convince the compiler that
// the variable is used in this location, to help control
// code motion and de-duplication that would result in a slowdown.
//#define FORCE_REFERENCE(var)  asm volatile( "" : : "r" (var) )


// from https://github.com/FastLED/FastLED/blob/master/lib8tion/scale8.h
//
// scale a value by a scale value that is in the range 0..255 but interpreted 0..1
inline uint8_t scale8u(uint8_t i, uint8_t scale) {
	return (i * scale + i) >> 8;
}

inline RGB scale8RGB(RGB color, uint8_t scale) {
	RGB rgb = {
		scale8u(color.red, scale),
		scale8u(color.green, scale),
		scale8u(color.blue, scale)};
	return rgb;
}

inline uint16_t scale16u8u(uint16_t i, uint8_t scale) {
	return (i * scale + i) >> 8;
}

inline int16_t scale16s8u(int16_t i, uint8_t scale) {
	return (i * scale + i) >> 8;
}

inline uint16_t scale16u(uint16_t i, uint16_t scale) {
	return (i * scale + i) >> 16;
}

// average for 8 bit signed integers
inline int8_t avg8s(int8_t i, int8_t j) {
	return (i + j + 1) >> 1;
}

// lerp for 8 bit signed integers. the interpolator r is interpreted as 0..255/256
inline int8_t lerp8s(int8_t a, int8_t b, uint8_t r) {
	if (b > a)
		return a + (uint8_t)(r * (uint8_t)(b - a) >> 8);
	return a - (uint8_t)(r * (uint8_t)(a - b) >> 8);
}

inline int clamp(int x, int minValue, int maxValue) {
	return x < minValue ? minValue : (x > maxValue ? maxValue : x);
}
