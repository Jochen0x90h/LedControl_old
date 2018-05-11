#pragma once

#include "Widget.h"
#include "Bitmap.h"


class Display : public Widget, public Bitmap {
public:
	
	Display(uint16_t width, uint16_t height);
	
	~Display() override;
	
	void update();

protected:

	void setState() override;
	void resetState() override;

	uint8_t * buffer;
	GLuint texture;
};
