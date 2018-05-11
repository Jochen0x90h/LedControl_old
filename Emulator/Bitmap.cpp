#include "Bitmap.h"


void Bitmap::clear() {
	int size = this->width * this->height >> 3;
	for (int i = 0; i < size; ++i)
		this->data[i] = 0;
	
	this->minX = 0;
	this->minY = 0;
	this->maxX = this->width;
	this->maxY = this->height;
}
	
void Bitmap::rectangle(int x, int y, int width, int height, Mode mode) {
	fillRectangle(x, y, width, 1, mode);
	fillRectangle(x, y + height - 1, width, 1, mode);
	fillRectangle(x, y + 1, 1, height - 2, mode);
	fillRectangle(x + width - 1, y + 1, 1, height - 2, mode);
}

void Bitmap::fillRectangle(int x, int y, int width, int height, Mode mode) {
	if (mode == KEEP)
		return;
	
	// clamp to border
	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x + width > this->width) {
		width = this->width - x;
	}
	if (y + height > this->height) {
		height = this->height - y;
	}
	
	
	int endRows = (y + height) & 7;
	
	uint8_t * page = &this->data[(y >> 3) * this->width];
	uint8_t * pageEnd = &this->data[((y + height) >> 3) * this->width];

	// first page
	{
		uint8_t mask = 0xff << (y & 7);
		if (page == pageEnd && endRows > 0) {
			mask &= 0xff >> (8 - endRows);
			endRows = 0;
		}
		for (int c = x; c < x + width; ++c) {
			switch (mode) {
			case CLEAR:
				page[c] &= ~mask;
				break;
			case FLIP:
				page[c] ^= mask;
				break;
			default:
				page[c] |= mask;
				break;
			}
		}
		page += this->width;
	}
	
	// full pages
	while (page < pageEnd) {
		for (int c = x; c < x + width; ++c) {
			switch (mode) {
			case CLEAR:
				page[c] &= 0;
				break;
			case FLIP:
				page[c] ^= 0xff;
				break;
			default:
				page[c] |= 0xff;
				break;
			}
		}
		page += this->width;
	}

	// last page
	if (endRows > 0) {
		uint8_t mask = 0xff >> (8 - endRows);
		for (int c = x; c < x + width; ++c) {
			switch (mode) {
			case CLEAR:
				page[c] &= ~mask;
				break;
			case FLIP:
				page[c] ^= mask;
				break;
			default:
				page[c] |= mask;
				break;
			}
		}
		page += this->width;
	}
	
	// update dirty region
	if (x < this->minX)
		this->minX = x;
	if (y < this->minY)
		this->minY = y;
	if (x + width > this->maxX)
		this->maxX = x + width;
	if (y + height > this->maxY)
		this->maxY = y + height;
}

void Bitmap::bitmap(int x, int y, int width, int height, uint8_t const * bitmap, Mode mode) {
	if (mode == KEEP)
		return;

	// number of bytes in a row of the bitmap
	int bitmapStride = (width + 7) >> 3;

	// clamp to border
	int o = 0;
	if (x < 0) {
		o = -x;
		width += x;
		x = 0;
	}
	if (y < 0) {
		bitmap += -y * bitmapStride;
		height += y;
		y = 0;
	}
	if (x + width > this->width) {
		width = this->width - x;
	}
	if (y + height > this->height) {
		height = this->height - y;
	}

	// bitmap
	for (int j = 0; j < height; ++j) {
		uint8_t * page = &this->data[((y + j) >> 3) * this->width];
		
		for (int i = 0; i < width; ++i) {
			if (bitmap[(o + i) >> 3] & (0x80 >> ((o + i) & 7))) {
				int c = i + x;
				uint8_t bit = 1 << ((y + j) & 7);
				switch (mode) {
				case CLEAR:
					page[c] &= ~bit;
					break;
				case FLIP:
					page[c] ^= bit;
					break;
				default:
					page[c] |= bit;
					break;
				}
			}
		}
		bitmap += bitmapStride;
	}
/*
	uint8_t * line = &this->data[y * this->width >> 3];
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			if (bitmap[(o + i) >> 3] & (0x80 >> ((o + i) & 7))) {
				int p = i + x;
				uint8_t bit = 0x80 >> (p & 7);
				switch (mode) {
				case CLEAR:
					line[p >> 3] &= ~bit;
					break;
				case FLIP:
					line[p >> 3] ^= bit;
					break;
				default:
					line[p >> 3] |= bit;
					break;
				}
			}
		}
		line += this->width >> 3;
		bitmap += bitmapStride;
	}
*/
	// update dirty region
	if (x < this->minX)
		this->minX = x;
	if (y < this->minY)
		this->minY = y;
	if (x + width > this->maxX)
		this->maxX = x + width;
	if (y + height > this->maxY)
		this->maxY = y + height;
}


int Bitmap::text(int x, int y, Font const & font, char const * text, Mode mode) {
	if (text == nullptr || *text == 0)
		return 0;
	for (; *text != 0; ++text) {
		unsigned char ch = *text;
		if (ch <= font.first || ch >= font.last)
			ch = ' ';
		Character const & character = font.characters[ch - font.first];
		
		bitmap(x, y, character.width, font.height, font.bitmap + character.offset, mode);
		
		x += character.width + 1;
	}
	return x;
}
