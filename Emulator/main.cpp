#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <glad/glad.h> // http://glad.dav1d.de/#profile=compatibility&specification=gl&api=gl%3D3.2&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&language=c
#include <GLFW/glfw3.h> // http://www.glfw.org/docs/latest/quick_guide.html
#define GLFONTSTASH_IMPLEMENTATION
//#include "glfontstash.h" // https://github.com/memononen/fontstash
//#include "linmath.h"
//#include <stdlib.h>
//#include <stdio.h>
#include "LedStrip.h"
#include "Display.h"
#include "Slider.h"

// fonts
#include "tahoma_8pt.h"


static void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
 /*   if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if(GLFW_PRESS == action)
            lbutton_down = true;
        else if(GLFW_RELEASE == action)
            lbutton_down = false;
    }

    if(lbutton_down) {
         // do your drag here
    }*/
}

//
//uint8_t pgm_read_byte(const uint8_t* value) {return *value;}
//uint16_t pgm_read_word(const uint16_t* value) {return *value;}

//#include "../Generator/exp16_32.h"



#define LED_COUNT 300

int colorIndex;
RGB colors[LED_COUNT];

void sendRGB(RGB const & color) {
	colors[colorIndex] = color;
	++colorIndex;
}

inline void sendColor(uint8_t red, uint8_t green, uint8_t blue) {
	sendRGB(makeRGB(red, green, blue));
}

// effects (need sendColor())
#include "Color.h"
#include "Rainbow.h"
#include "MovingLight.h"
#include "Spring.h"
#include "Summer.h"
#include "Autumn.h"
#include "Winter.h"
#include "Plasma.h"
EffectInfo const effectInfos[] = {
	EFFECT(Color),
	EFFECT(Rainbow),
	EFFECT(Spring),
	EFFECT(Summer),
	EFFECT(Autumn),
	EFFECT(Winter),
	EFFECT(Plasma),
	EFFECT(MovingLight),
};


class LayoutManager {
public:

	void add(Widget * widget) {
		this->widgets.push_back(widget);
	}
	
	void doMouse(GLFWwindow * window) {
		// get window size
		int windowWidth, windowHeight;
		glfwGetWindowSize(window, &windowWidth, &windowHeight);

		double x;
        double y;
        glfwGetCursorPos(window, &x, &y);
		bool leftDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		//std::cout << "x " << x << " y " << y << " down " << leftDown << std::endl;
		if (leftDown) {
			if (this->activeWidget == nullptr) {
				for (Widget * widget : this->widgets) {
					if (widget->contains(x / windowWidth, 1.0f - y / windowHeight))
						this->activeWidget = widget;
				}
			}
			if (this->activeWidget != nullptr)
				this->activeWidget->touch(x / windowWidth, y / windowHeight);
		} else {
			this->activeWidget = nullptr;
		}
	}
	
	void draw() {
		for (Widget * widget : this->widgets) {
			widget->draw();
		}
	}
	
	std::vector<Widget *> widgets;
	Widget* activeWidget = nullptr;
};

int main(void) {
	GLFWwindow * window;
	
	// init
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	
	// create window and OpenGL context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(640, 480, "LedControl", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	
	// no v-sync
	glfwSwapInterval(0);

	LayoutManager layoutManager;

	// led strip and rgb graphs
	LedStrip ledStrip(LED_COUNT);
	layoutManager.add(&ledStrip);
	ledStrip.x1 = 0.0f;
	ledStrip.y1 = 0.8f;
	ledStrip.x2 = 1.0f;
	ledStrip.y2 = 1.0f;
	
	float y = ledStrip.y1 - 0.2f;

	// display
	Display display(128, 64);
	layoutManager.add(&display);
	display.x1 = 0.5;
	display.y1 = y - 0.2f;
	display.x2 = 0.9f;
	display.y2 = y;

	// effect selector slider
	int const effectCount = sizeof(effectInfos) / sizeof(EffectInfo);
	ParameterInfo const effectSliderInfo = {"Selector", 0, effectCount - 1, 1, 0};
	Slider * effectSlider = new Slider();
	effectSlider->setParameterInfo(&effectSliderInfo, 0);
	layoutManager.add(effectSlider);
	effectSlider->x1 = 0.0f;
	effectSlider->y1 = y - 0.05f;
	effectSlider->x2 = 0.3f;
	effectSlider->y2 = y;
	y -= 0.06f;

	// brightness and maximum number of additional parameters
	ParameterInfo brightnessInfo = PARAMETER("Brightness", 25, 255, 8, 255);
	int const maxParameterCount = 8;

	// effect parameter sliders
	Slider * parameterSliders[maxParameterCount + 1];
	for (int i = 0; i < maxParameterCount + 1; ++i) {
		parameterSliders[i] = new Slider();
		layoutManager.add(parameterSliders[i]);
		parameterSliders[i]->x1 = 0.0f;
		parameterSliders[i]->y1 = y - 0.05f;
		parameterSliders[i]->x2 = 0.3f;
		parameterSliders[i]->y2 = y;
		y -= 0.06f;
	}

	// set effect parameters to initial values
	uint8_t brightness = 255;
	uint8_t parameters[effectCount][maxParameterCount];
	for (int effectIndex = 0; effectIndex < effectCount; ++effectIndex) {
		EffectInfo const & effectInfo = effectInfos[effectIndex];
		for (int i = 0; i < effectInfo.parameterCount; ++i) {
			parameters[effectIndex][i] = effectInfo.parameterInfos[i].initValue;
		}
	}

	// set parameter info for brightness (is shared across all effects)
	parameterSliders[0]->setParameterInfo(&brightnessInfo, brightness);

	// current effect
	EffectInfo const * effectInfo = nullptr;
	uint8_t effectData[32];
	int parameterIndex = 0;
	
	// loop
	int frameCount = 0;
	auto start = std::chrono::steady_clock::now();
	while (!glfwWindowShouldClose(window)) {
		auto frameStart = std::chrono::steady_clock::now();

		// get frame buffer size
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// mouse
		layoutManager.doMouse(window);
		
		// check if effect has changed
		uint8_t effectIndex = effectSlider->getValue();
		if (effectInfo != &effectInfos[effectIndex]) {
			effectInfo = &effectInfos[effectIndex];

			for (int i = 0; i < effectInfo->parameterCount; ++i) {
				parameterSliders[i+1]->setParameterInfo(&effectInfo->parameterInfos[i], parameters[effectIndex][i]);
			}
			for (int i = effectInfo->parameterCount; i < maxParameterCount; ++i) {
				parameterSliders[i+1]->setParameterInfo(nullptr, 0);
			}

			// init new effect
			effectInfo->init(effectData, LED_COUNT);
			
			// index of parameter to display
			parameterIndex = 0;
		}

		// check if parameter sliders have changed
		if (parameterSliders[0]->getValue() != brightness) {
			brightness = parameterSliders[0]->getValue();

			// index of parameter to display (brightness)
			parameterIndex = 0;
		}
		for (int i = 0; i < effectInfo->parameterCount; ++i) {
			uint8_t value = parameterSliders[i+1]->getValue();
			if (value != parameters[effectIndex][i]) {
				parameters[effectIndex][i] = value;

				// index of parameter to display
				parameterIndex = i+1;
			}
		}

		// display
		display.clear();
		{
			// effect name
			int y = 10;
			int len = tahoma_8pt.calcWidth(effectInfo->name);
			display.text((display.width - len) >> 1, y, tahoma_8pt, effectInfo->name, Bitmap::SET);
			
			// parameter name and value
			y = 30;
			ParameterInfo const * parameterInfo = parameterSliders[parameterIndex]->getParameterInfo();
			int value = parameterSliders[parameterIndex]->getValue();
			int w = 124 * (value - parameterInfo->minValue) / (parameterInfo->maxValue - parameterInfo->minValue);
			display.fillRectangle(2, y + 2, w, 13, Bitmap::SET);
			len = tahoma_8pt.calcWidth(parameterInfo->name);
			display.text(((display.width - len) >> 1), y + 3, tahoma_8pt, parameterInfo->name, Bitmap::FLIP);
			display.rectangle(0, y, 128, 17, Bitmap::SET);
		}
		display.update();

		// run effect
		colorIndex = 0;
		effectInfo->run(effectData, LED_COUNT, brightness, parameters[effectIndex]);
		ledStrip.set(colors);
		
		// draw on screen
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		layoutManager.draw();
		
		glfwSwapBuffers(window);
		glfwPollEvents();

		auto now = std::chrono::steady_clock::now();
		std::this_thread::sleep_for(std::chrono::milliseconds(9) - (now - frameStart));
		
		++frameCount;
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
		if (duration.count() > 1000) {
			std::cout << frameCount * 1000 / duration.count() << "fps" << std::endl;
			
			frameCount = 0;
			start = std::chrono::steady_clock::now();
		}
	}
	//effect->~Effect();
	
	// cleanup
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
