#pragma once


typedef struct {
	// red
	uint8_t red;
	
	// green
	uint8_t green;
	
	// blue
	uint8_t blue;
} RGB;

inline RGB makeRGB(uint8_t r, uint8_t g, uint8_t b) {
	RGB rgb = {r, g, b};
	return rgb;
}

typedef struct {
	// hue 0..2047
	uint16_t hue;
	
	// saturation 0..255
	uint8_t saturation;

	// value 0..255
	uint8_t value;
} HSV;

inline HSV makeHSV(uint16_t h, uint8_t s, uint8_t v) {
	HSV hsv = {h, s, v};
	return hsv;
}
