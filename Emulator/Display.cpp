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
		"}\n"), Bitmap(width, height, new uint8_t[width * height >> 3]), buffer(new uint8_t[width * height]) {

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
	delete [] this->data;
	delete [] this->buffer;
}
	
void Display::update() {
	for (int j = 0; j < this->height; ++j) {
		uint8_t * b = &this->buffer[this->width * (this->height - 1 - j)];
		for (int i = 0; i < this->width; ++i) {
			bool bit = (this->data[i + this->width * (j >> 3)] & (1 << (j & 7))) != 0;
			b[i] = bit ? 255 : 48;
		}
	}
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->width, this->height, GL_RED, GL_UNSIGNED_BYTE, this->buffer);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void Display::setState() {
	glBindTexture(GL_TEXTURE_2D, this->texture);
}

void Display::resetState() {
	glBindTexture(GL_TEXTURE_2D, 0);
}
