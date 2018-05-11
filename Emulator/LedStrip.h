#pragma once

#include "Widget.h"


class LedStrip : public Widget {
public:

	LedStrip(int ledCount);

	///
	/// set rgb data
	void set(std::vector<RGB> colors);
	void set(RGB* colors);
	
protected:

	void setState() override;
	void resetState() override;

	int ledCount;
	GLuint texture;
};
