#pragma once

#include <stdint.h>
#include "Types.h"

// puts a variable into flash memory
#define FLASH const __flash


// from https://github.com/FastLED/FastLED/blob/master/hsv2rgb.cpp
//
// Sometimes the compiler will do clever things to reduce
// code size that result in a net slowdown, if it thinks that
// a variable is not used in a certain location.
// This macro does its best to convince the compiler that
// the variable is used in this location, to help control
// code motion and de-duplication that would result in a slowdown.
//#define FORCE_REFERENCE(var)  asm volatile( "" : : "r" (var) )


inline int16_t mul8s(int8_t a, int8_t b) {
	//return a * b;

	int16_t result;
	asm volatile (
		// multiply 8-bit value by 8-bit scale, giving 16-bit r1,r0
		"muls %[a], %[b] \n\t"
		"mov %A[result], r0 \n\t"
		"mov %B[result], r1 \n\t"
		// restore r1 to 0, it's expected to always be that
		"clr __zero_reg__ \n\t"		
		: [result] "=w" (result)
		: [a] "a" (a), [b] "a" (b)
		: "r0"); // clobbers r0
	return result;
}


// from https://github.com/FastLED/FastLED/blob/master/lib8tion/scale8.h
// scale a value by a scale value that is in the range 0..255 but interpreted 0..1
inline uint8_t scale8u(uint8_t value, uint8_t scale) {
	//return (value * scale + value) >> 8;

	asm volatile (
		// multiply 8-bit value by 8-bit scale, giving 16-bit r1,r0
		"mul %[value], %[scale] \n\t"
		// add value to r0, possibly setting the carry flag
		"add r0, %[value] \n\t"
		// load 0 into result (note, this does _not_ touch any flags)
		"ldi %[value], 0 \n\t"
		// add upper part of multiplication and carry flag into result
		"adc %[value], r1 \n\t"
		// restore r1 to 0, it's expected to always be that
		"clr __zero_reg__ \n\t"		
		: [value] "+a" (value)
		: [scale] "a" (scale)
		: "r0"); // clobbers r0
	return value;
}

inline RGB scale8RGB(RGB color, uint8_t scale) {
/*	RGB result = {
		scale8u(color.red, scale),
		scale8u(color.green, scale),
		scale8u(color.blue, scale)};
	return result;*/

	uint8_t red = color.red;
	uint8_t green = color.green;
	uint8_t blue = color.blue;
	asm volatile (
		"mul %[red], %[scale] \n\t"
		"add r0, %[red] \n\t"
		"ldi %[red], 0 \n\t"
		"adc %[red], r1 \n\t"

		"mul %[green], %[scale] \n\t"
		"add r0, %[green] \n\t"
		"ldi %[green], 0 \n\t"
		"adc %[green], r1 \n\t"

		"mul %[blue], %[scale] \n\t"
		"add r0, %[blue] \n\t"
		"ldi %[blue], 0 \n\t"
		"adc %[blue], r1 \n\t"

		// restore r1 to 0, it's expected to always be that
		"clr __zero_reg__ \n\t"		
		: [red] "+a" (red), [green] "+a" (green), [blue] "+a" (blue)
		: [scale] "a" (scale)
		: "r0"); // clobbers r0
	color.red = red;
	color.green = green;
	color.blue = blue;
	return color;
}

inline uint16_t scale16u8u(uint16_t i, uint8_t scale) {
	return ((uint32_t)i * scale + i) >> 8;
}

inline int16_t scale16s8u(int16_t i, uint8_t scale) {
	return ((int32_t)i * scale + i) >> 8;
}

inline uint16_t scale16u(uint16_t i, uint16_t scale) {
	return ((uint32_t)i * scale + i) >> 16;
}

// average for 8 bit signed integers
inline int8_t avg8s(int8_t i, int8_t j) {
	return (i + j + 1) >> 1;
}

// lerp for 8 bit signed integers. the interpolator r is interpreted as [0, 255]/256
inline int8_t lerp8s(int8_t a, int8_t b, uint8_t r) {
	if (b > a)
		return a + (uint8_t)(r * (uint8_t)(b - a) >> 8);
	return a - (uint8_t)(r * (uint8_t)(a - b) >> 8);
}

inline int clamp(int x, int minValue, int maxValue) {
	return x < minValue ? minValue : (x > maxValue ? maxValue : x);
}
