#include "Display.h"


Display::Display(uint16_t width, uint16_t height)
		: Widget(
		// fragment shader
		"#version 150\n"
		"uniform sampler2D tex;\n"
		"in vec2 uv;\n"
		"out vec4 pixel;\n"
		"void main() {\n"
			"pixel = texture(tex, uv).xxxw;\n"
		"}\n"),
		bitmap(width, height, new uint8_t[width * height >> 3]), buffer(new uint8_t[width * height]) {

	setTextureIndex("text", 0);

	// create texture
	glGenTextures(1, &this->texture);
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

}

Display::~Display() {
	delete [] this->bitmap.data;
	delete [] this->buffer;
}
	
void Display::update() {
	int width = this->bitmap.width;
	int height = this->bitmap.height;
	for (int j = 0; j < height; ++j) {
		uint8_t * b = &this->buffer[width * (height - 1 - j)];
		for (int i = 0; i < width; ++i) {
			// data layout: rows of 8 pixels where each byte describes a column in each row
			// this would be the layout of a 16x16 display where each '|' is one byte
			// ||||||||||||||||
			// ||||||||||||||||
			bool bit = (this->bitmap.data[i + width * (j >> 3)] & (1 << (j & 7))) != 0;
			b[i] = bit ? 255 : 48;
		}
	}
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, this->buffer);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void Display::setState() {
	glBindTexture(GL_TEXTURE_2D, this->texture);
}

void Display::resetState() {
	glBindTexture(GL_TEXTURE_2D, 0);
}
