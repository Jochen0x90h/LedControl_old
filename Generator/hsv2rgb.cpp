#include <fstream>
#include <cmath>
#include <boost/filesystem.hpp>
#include "hsv2rgb.h"


struct RGB {
	// linear RGB for display on LEDs with gamma of 1
	uint8_t r;
	uint8_t g;
	uint8_t b;

	RGB() = default;
	RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
	
	RGB correctGamma(float gamma) const {
		return RGB(
			uint8_t(std::round(std::pow(r / 255.0f, gamma) * 255.0f)),
			uint8_t(std::round(std::pow(g / 255.0f, gamma) * 255.0f)),
			uint8_t(std::round(std::pow(b / 255.0f, gamma) * 255.0f)));
	}
};

// original from https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
const RGB hueColors[8] = {
	{255, 0, 0}, // red
	{170, 85, 0}, // orange
//	{255, 106, 0}, // orange
	{170, 170, 0}, // yellow
	{0, 255, 0}, // green
	{0, 170, 85}, // aqua
//	{0, 170, 170}, // aqua
	{0, 0, 255}, // blue
	{85, 0, 170}, // purple
	{170, 0, 85} // pink
};

const RGB autumnColors[4] = {
	//{139, 69, 19}, // brown
	{255, 0, 0}, // red
	{170, 85, 0}, // orange
	{170, 170, 0}, // yellow
	{0, 255, 0}, // green
};

int highestBit(int x) {
	int i = 1;
	while (x > 1) {
		x >>= 1;
		i <<= 1;
	}
	return i;
}

class Gradient {
public:
	std::string name;
	int length;
	const RGB *colors;
	bool wrap;
	std::ofstream f;


	Gradient(boost::filesystem::path outputPath, const std::string name, int length, const RGB *colors, bool wrap)
			: name(name), length(length), colors(colors), wrap(wrap), f((outputPath / (name + ".h")).string()) {
		f << "#pragma once" << std::endl;
		f << "INLINE RGB " << name << "(uint8_t section, uint8_t x) {" << std::endl;
		level("\t", length, 0);
		f << "}" << std::endl;
	}

	void level(const std::string& indent, int length, int index) {
		if (length > 1) {
			/*int mid = length / 2;
			f << indent << "if (section < " << index + mid << ") {" << std::endl;
			level(indent + '\t', mid, index);
			f << indent << "} else {" << std::endl;
			level(indent + '\t', length - mid, index + mid);
			f << indent << "}" << std::endl;*/
			
			int mid = highestBit(length - 1);
			f << indent << "if ((section & " << mid << ") == 0) {" << std::endl;
			level(indent + '\t', mid, index);
			f << indent << "}" << std::endl;
			level(indent, length - mid, index + mid);
			
		} else {
			RGB color1 = this->colors[index];
			RGB color2 = this->colors[this->wrap ? (index + 1) % this->length : index + 1];
			f << indent << "// " << index << ": " << int(color1.r) << " " << int(color1.g) << " " << int(color1.b) << std::endl;
			
			f << indent << "return RGB(";
			interpolate(indent, 'r', color1.r, color2.r); f << ", ";
			interpolate(indent, 'g', color1.g, color2.g); f << ", ";
			interpolate(indent, 'b', color1.b, color2.b);
			f << ");" << std::endl;
		}
	}

	void interpolate(const std::string &indent, char channel, int value1, int value2) {
		f << /*indent << channel << " = " <<*/ value1;
		if (value2 != value1)
			f << (value2 > value1 ? " + " : " - ") << "(x * " << std::abs(value2 - value1) << " >> 8)";
		//f << ';' << std::endl;
	}
};


// main function
void hsv2rgb(boost::filesystem::path outputPath) {
	Gradient(outputPath, "hue", 8, hueColors, true);
	Gradient(outputPath, "autumnGradient", 4, autumnColors, true);
}
