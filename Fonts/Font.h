#pragma once

#include <stdint.h>


struct Character {
	//int code;
	
	// character width
	uint8_t width;
	
	// character offset in bitmap data
	uint16_t offset;
};

struct Font {
	// common character height
	uint8_t height;

	// space between characters
	uint8_t space;

	// first/last characters in font
	unsigned char first;
	unsigned char last;
	
	// characters
	Character const * characters;

	// bitmap data
	uint8_t const * bitmap;
	
	int calcWidth(char const * text);
};
