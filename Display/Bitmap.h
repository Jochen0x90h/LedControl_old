#pragma once

#include "Font.h"


class Bitmap {
public:
	
	Bitmap(uint16_t width, uint16_t height, uint8_t * data)
			: width(width), height(height), data(data), minX(255), minY(255), maxX(0), maxY(0) {
	}
	

	// draw methods

	enum Mode {
		CLEAR = 0,
		KEEP = 1,
		FLIP = 2,
		SET = 3
	};

	void clear();
	
	void drawRectangle(int x, int y, int width, int height, Mode mode);

	void fillRectangle(int x, int y, int width, int height, Mode mode);
	
	void drawGlyph(int x, int y, int width, int height, uint8_t const * data, Mode mode);

	int drawText(int x, int y, Font const & font, char const * text, Mode mode);


	uint16_t const width;
	uint16_t const height;

	// bitmap data, data layout: rows of 8 pixels where each byte describes a column in each row
	// this would be the layout of a 16x16 display where each '|' is one byte
	// ||||||||||||||||
	// ||||||||||||||||
	uint8_t * const data;

protected:

	// dirty area
	uint16_t minX;
	uint16_t minY;
	uint16_t maxX;
	uint16_t maxY;
};

/*
struct Bitmap {
	uint16_t const width;
	uint16_t const height;
	
	// bitmap data, data layout: rows of 8 pixels where each byte describes a column in each row
	// this would be the layout of a 16x16 display where each '|' is one byte
	// ||||||||||||||||
	// ||||||||||||||||
	uint8_t * const data;

	// dirty area
	uint16_t minX;
	uint16_t minY;
	uint16_t maxX;
	uint16_t maxY;


	enum Mode {
		CLEAR = 0,
		KEEP = 1,
		FLIP = 2,
		SET = 3
	};
};

// clear bitmap
void clearBitmap(Bitmap * bitmap);

// draw rectangle outline
void drawRectangle(Bitmap * bitmap, int x, int y, int width, int height, Bitmap::Mode mode);

// draw filled rectangle
void fillRectangle(Bitmap * bitmap, int x, int y, int width, int height, Bitmap::Mode mode);

// draw glyph bitmap (horizontal bit order)
void drawGlyph(Bitmap * bitmap, int x, int y, int width, int height, uint8_t const * data, Bitmap::Mode mode);

// draw text using given font
int drawText(Bitmap * bitmap, int x, int y, Font const * font, char const * text, Bitmap::Mode mode);
*/
