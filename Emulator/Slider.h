#pragma once

#include "Widget.h"
#include "Effect.h"


class Slider : public Widget {
public:
	
	Slider();

	~Slider() override;

	void setParameterInfo(const ParameterInfo * parameterInfo, uint8_t value);
	
	uint8_t getValue() {return this->value;}

	void touch(float x, float y) override;
	
protected:
	
	void setState() override;
	
	const ParameterInfo * parameterInfo;
	uint8_t value;
	GLuint valueLocation;
};
