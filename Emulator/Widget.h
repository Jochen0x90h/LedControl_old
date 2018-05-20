#pragma once

#include <glad/glad.h>
#include "Types.h"


class Widget {
public:
	
	// bounding box
	float x1;
	float y1;
	float x2;
	float y2;
	
	Widget(const char *fragmentShaderSource);

	virtual ~Widget();

	///
	/// check if widget contains the given point
	bool contains(float x, float y);

	///
	/// called when the widget is "touched" at given position (mouse is pressed/dragged)
	virtual void touch(float x, float y);

	///
	/// draw component
	virtual void draw();
	
protected:
	void setTextureIndex(char const * name, int index);
	
	// derived classes can set additional opengl state
	virtual void setState();

	// derived classes can reset additional opengl state
	virtual void resetState();
	
	static GLuint createShader(GLenum type, const char* code);

	GLuint program;
	GLuint scaleLocation;
	GLuint offsetLocation;
	GLuint vertexBuffer;
	GLuint vertexArray;
};
