#pragma once

#include "Widget.h"
#include "Effect.h"


class Slider : public Widget {
public:
	
	Slider();

	~Slider() override;

	void setParameterInfo(const ParameterInfo * parameterInfo, uint8_t value);
	ParameterInfo const * getParameterInfo() {return this->parameterInfo;}
	
	uint8_t getValue() {return this->value;}

	void touch(float x, float y) override;
	
	void draw() override;
	
protected:
	
	void setState() override;
	
	ParameterInfo const * parameterInfo;
	uint8_t value;
	GLuint valueLocation;
};
