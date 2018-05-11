#pragma once

#define INLINE inline
#define TABLE8(name) const uint8_t name[]
#define TABLE16(name) const uint16_t name[]
#define READ8(table, x) table[x]
#define READ16(table, x) table[x]

//#define TABLE8(type, name) const uint8_t name[] PROGMEM
//#define TABLE16(type, name) const uint16_t name[] PROGMEM
//#define READ8(table, x) pgm_read_byte(&table[x])
//#define READ16(table, x) pgm_read_word(&table[x])


struct RGB {
	uint8_t r;
	uint8_t g;
	uint8_t b;

	RGB() = default;
	RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
};

struct HSV {
	// 0..2047
	uint16_t h;
	uint8_t s;
	uint8_t v;
	
	HSV() = default;
	HSV(uint16_t h, uint8_t s, uint8_t v) : h(h), s(s), v(v) {}
};
