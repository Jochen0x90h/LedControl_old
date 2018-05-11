#pragma once

#include <stdint.h>
#include "Types.h"


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
inline uint8_t scale8(uint8_t i, uint8_t scale) {
	return (i * scale + i) >> 8;
}
inline uint16_t scale16_8(uint16_t i, uint8_t scale) {
	return (i * scale + i) >> 8;
}
inline uint16_t scale16(uint16_t i, uint16_t scale) {
	return (i * scale + i) >> 16;
}

// average for 8 bit signed integers
inline int8_t avg8s(int8_t i, int8_t j) {
	return (i + j + 1) >> 1;
}

// lerp for 8 bit signed integers. the interpolator r is interpreted as 0..255/256
inline int8_t lerp8s(int8_t a, int8_t b, uint8_t r) {
	if (b > a)
		return a + uint8_t(r * uint8_t(b - a) >> 8);
	return a - uint8_t(r * uint8_t(a - b) >> 8);
}

/*
// hsv to rgb using rainbow scheme
// see https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
#include "../Generator/hue.h"
inline RGB hsv2rgb(const HSV& hsv) {
	// convert hue to rgb
	RGB rgb = hue(hsv.h >> 8, hsv.h);
	
	/ *uint16_t hue = hsv.h;
    uint8_t offset = uint8_t(hue); // 0..255
    uint8_t r, g, b;
	#include "../Generator/hsv2rgb.inc"
* /
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
*/

/*
static const uint8_t permuteTable[] = { 151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,151
};
inline uint8_t permute8(uint8_t x) {return permuteTable[x];}

// helper function: calculate bumps of varying height in the range -128..127
inline int8_t bump8s(uint8_t i, uint8_t x) {
	int8_t hash = permute8(i);
	int8_t y = uint8_t(x * x >> 7) - uint8_t(x + x);
	
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
	uint8_t f = uint8_t(x);

	// bump for position i and negative bump for next position
	int8_t g1 = bump8s(i, f);
	int8_t g2 = ~bump8s(i + 1, f);
	
	// interpolate between the bumps
	return lerp8s(g1, g2, f);
}
*/

// smoothstep
// f(x) = ax^3 + bx^2 + cx + d
// f'(x) = 3ax^2 + 2bx + c
// f(0) = 0 -> d = 0
// f'(0) = 0 -> c = 0
// f(1) = 1 -> a + b = 1
// f'(1) = 0 -> 3a + 2b = 0
// a = -2
// b = 3
// x^2(x - 1,5)


/*
#define FADE(x) scale16(x,x)
#define LERP(a,b,u) lerp15by16(a,b,u)
inline int16_t inoise16_raw(uint32_t x) {
	// Find the unit cube containing the point
	uint8_t X = x>>16;

	// Hash cube corner coordinates
	uint8_t A = P(X);
	uint8_t AA = P(A);
	uint8_t B = P(X+1);
	uint8_t BA = P(B);

	// Get the relative position of the point in the cube
	uint16_t u = x & 0xFFFF;

	// Get a signed version of the above for the grad function
	int16_t xx = (u >> 1) & 0x7FFF;
	uint16_t N = 0x8000L;

	u = FADE(u);

	int16_t ans = LERP(grad16(P(AA), xx), grad16(P(BA), xx - N), u);

	return ans;
}
*/
inline int clamp(int x, int minValue, int maxValue) {
	return x < minValue ? minValue : (x > maxValue ? maxValue : x);
}
