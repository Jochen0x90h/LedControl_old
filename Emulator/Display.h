#pragma once

#include "Widget.h"
#include "Bitmap.h"


class Display : public Widget {
public:
	
	Display(uint16_t width, uint16_t height);
	
	~Display() override;
	
	void update();

	Bitmap bitmap;

protected:

	void setState() override;
	void resetState() override;

	// texture buffer for copying bitmap contents into the texture
	uint8_t * buffer;
	GLuint texture;
};
