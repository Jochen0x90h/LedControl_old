#include <vector>
#include <stdexcept>
#include "LedStrip.h"


LedStrip::LedStrip(int ledCount) : Widget(
		// fragment shader
		"#version 150\n"
		"uniform sampler2D tex;\n"
		"in vec2 uv;\n"
		"out vec4 pixel;\n"
		"void main() {\n"
			"vec4 color = texture(tex, uv);\n"
			// use sqrt(color) to simulate linear led's on screen with gamma = 2.0
			"pixel = max(1.0 - abs(color - uv.y * 2.0 + 0.05) * 20, 0) + (uv.y > 0.6 ? sqrt(color) : vec4(0));\n"
		"}\n"), ledCount(ledCount) {

	setTextureIndex("text", 0);
	//glUseProgram(this->program);
	//glUniform1i(glGetUniformLocation(this->program, "tex"), 0);
	//glUseProgram(0);

	// create texture
	glGenTextures(1, &this->texture);
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ledCount, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void LedStrip::set(std::vector<RGB> colors) {
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, colors.size(), 1, GL_RGB, GL_UNSIGNED_BYTE, colors.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void LedStrip::set(RGB* colors) {
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->ledCount, 1, GL_RGB, GL_UNSIGNED_BYTE, colors);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void LedStrip::setState() {
	glBindTexture(GL_TEXTURE_2D, this->texture);
}

void LedStrip::resetState() {
	glBindTexture(GL_TEXTURE_2D, 0);
}
