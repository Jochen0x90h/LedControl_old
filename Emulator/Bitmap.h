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
	
	void rectangle(int x, int y, int width, int height, Mode mode);

	void fillRectangle(int x, int y, int width, int height, Mode mode);
	
	void bitmap(int x, int y, int width, int height, uint8_t const * data, Mode mode);

	int text(int x, int y, Font const & font, char const * text, Mode mode);


	uint16_t const width;
	uint16_t const height;
	uint8_t * const data;

protected:

	uint16_t minX;
	uint16_t minY;
	uint16_t maxX;
	uint16_t maxY;
};
