#include <cmath>
#include "Util.h"
#include "Slider.h"


Slider::Slider() : Widget(
		"#version 150\n"
		"uniform float value;\n"
		"in vec2 uv;\n"
		"out vec4 pixel;\n"
		"void main() {\n"
		"    pixel = uv.x < value ? vec4(0.7, 0.7, 0.7, 1) : vec4(0.2, 0.2, 0.2, 1);\n"
		"}\n"), value(0) {
	
	// get uniform locations
	this->valueLocation = glGetUniformLocation(this->program, "value");
}

Slider::~Slider() {
}

void Slider::setParameterInfo(const ParameterInfo * parameterInfo, uint8_t value) {
	this->parameterInfo = parameterInfo;

	// set initial value
	this->value = value;
}

void Slider::touch(float x, float y) {
	if (this->parameterInfo == nullptr)
		return;
		
	float value = (x - this->x1) / (this->x2 - this->x1);
	this->value = clamp(int(std::round(value * (this->parameterInfo->maxValue - this->parameterInfo->minValue)))
			+ this->parameterInfo->minValue, this->parameterInfo->minValue, this->parameterInfo->maxValue);
}

void Slider::draw() {
	if (this->parameterInfo != nullptr)
		Widget::draw();
}

void Slider::setState() {
	// set uniforms
	glUniform1f(this->valueLocation, float(this->value - this->parameterInfo->minValue)
		/ (float)(this->parameterInfo->maxValue - this->parameterInfo->minValue));
}
