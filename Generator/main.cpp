#include <fstream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>


void cos(boost::filesystem::path outputPath) {
	std::ofstream f((outputPath / "cos8u10.h").string());
	f << "#pragma once" << std::endl;

	// unsigned cosine with 8 bit output and 10 bit input, shifted to positive values
	f << "uint16_t FLASH cos8Table[] = {" << std::endl;
	for (int j = 0; j < 64; ++j) {
		f << "\t";
		for (int i = 0; i < 16; ++i) {
			double x = double(i + 16*j) / 512.0 * M_PI;
			//int y = (i | j) == 0 ? 255 : 0;
			int y = int(round((cos(x) + 1.0) * 127.5));
			f << y << ", ";
		}
		f << std::endl;
	}	
	f << "};" << std::endl;
	f << "inline uint8_t cos8u10(uint16_t x) {return cos8Table[x];}" << std::endl;
}

void exp(boost::filesystem::path outputPath) {
	{
		// write exp table with 5 bit input and 16 bit output
		std::ofstream f((outputPath / "exp16u5.h").string());
		f << "#pragma once" << std::endl;
		f << "uint16_t FLASH exp16u5Table[] = {" << std::endl;
		f << "\t0";
		for (int i = 1; i < 32; ++i) {
			// x^31 = 65535 -> x = 65535^(1/31)
			int y = int(round(pow(65535.0, i / 31.0)));
			f << ", " << y;
		}
		f << std::endl;
		f << "};" << std::endl;
		f << "inline uint16_t exp16u5(uint8_t x) {return exp16u5Table[x];}" << std::endl;
	}
	{
		// write exp table with 8 bit input and 16 bit output
		std::ofstream f((outputPath / "exp16u8.h").string());
		f << "#pragma once" << std::endl;
		f << "uint16_t FLASH exp16u8Table[] = {" << std::endl;
		f << "\t0";
		for (int i = 1; i < 256; ++i) {
			// x^255 = 65535 -> x = 65535^(1/255)
			int y = int(round(pow(65535.0, i / 255.0)));
			f << ", " << y;
		}
		f << std::endl;
		f << "};" << std::endl;
		f << "inline uint16_t exp16u8(uint8_t x) {return exp16u8Table[x];}" << std::endl;
	}
}

void permute(boost::filesystem::path outputPath) {
	// generate permute table
	srand(1337);
	int permute[256];
	for (int i = 0; i < 256; ++i) {
		permute[i] = i;
	}
	for (int i = 0; i < 10000; ++i) {
		int x = rand() & 0xff;
		int y = rand() & 0xff;
		std::swap(permute[x], permute[y]);
	}

	// write permutation table and function
	std::ofstream f((outputPath / "permute8.h").string());
	f << "#pragma once" << std::endl;
	f << "uint8_t FLASH permute8Table[] = {" << std::endl;
	f << "\t";
	for (int i = 0; i < 256; ++i) {
		if (i != 0)
			f << ", ";
		f << permute[i];
	}
	f << std::endl;
	f << "};" << std::endl;
	f << "inline uint8_t permute8(uint8_t x) {return permute8Table[x];}" << std::endl;
}


struct IndexColor {
	int index;
	
	int red;
	int green;
	int blue;

	IndexColor() = default;
	IndexColor(int index, uint8_t red, uint8_t green, uint8_t blue)
		: index(index), red(red), green(green), blue(blue) {}
};

inline int toInt(std::string const & s) {
	int base = 10;
	int i = 0;
	if (boost::starts_with(s, "0x")) {
		base = 16;
		i = 2;
	}
	
	int value = 0;
	for (; i < s.length(); ++i) {
		value *= base;
		char ch = s[i];
		
		if (ch >= '0' && ch <= '9') {
			value += ch - '0';
		} else if (base == 16) {
			if (ch >= 'A' && ch <= 'F')
				value += ch - 'A' + 10;
			else if (ch >= 'a' && ch <= 'f')
				value += ch - 'a' + 10;
		}
	}
	return value;
}


void interpolate(std::ofstream & f, const std::string &indent, int index1, int index2, int value1, int value2) {
	f << value1;
	if (value2 != value1) {
		f << (value2 > value1 ? " + " : " - ");
		f << '(';
		if ((index1 & 0xff) == 0)
			f << "(uint8_t)x";
		else
			f << "(uint8_t)(x - " << index1 << ")";
		f << " * " << std::abs(value2 - value1);
		int d = index2 - index1;
		int shift = 0;
		while (d > 1) {
			++shift;
			d >>= 1;
		}
		f << " >> " << shift;
		f << ')';
	}
}

void subdivide(std::ofstream & os, const std::string& indent, std::vector<IndexColor> const & palette, int begin, int end) {
	if (end - begin > 1) {
		int mid = (begin + end) / 2;
		
		os << indent << "if (x < 0x" << std::hex << palette[mid].index << std::dec << ") {" << std::endl;
		subdivide(os, indent + '\t', palette, begin, mid);
		os << indent << "} else {" << std::endl;
		subdivide(os, indent + '\t', palette, mid, end);
		os << indent << "}" << std::endl;
	} else {
		// add index as comment
		os << indent << "// 0x" << std::hex << palette[begin].index << std::dec << std::endl;

		// interpolate colors
		IndexColor color1 = palette[begin];
		IndexColor color2 = palette[begin + 1];
		os << indent << "color.red = "; interpolate(os, indent, color1.index, color2.index, color1.red, color2.red); os << ";" << std::endl;
		os << indent << "color.green = "; interpolate(os, indent, color1.index, color2.index, color1.green, color2.green); os << ";" << std::endl;
		os << indent << "color.blue = "; interpolate(os, indent, color1.index, color2.index, color1.blue, color2.blue); os << ";" << std::endl;
	}
}

void palette(boost::filesystem::path path, boost::filesystem::path outputPath) {
	std::vector<IndexColor> palette;

	// read palette
	std::ifstream is(path.string());
	std::string line;
	while (std::getline(is, line)) {
		// remove comment
		size_t pos = line.find("//");
		if (pos != std::string::npos)
			line = line.substr(0, pos);
		
		// trim
		boost::algorithm::trim(line);
		
		// split
		std::vector<std::string> elements;
		boost::split(elements, line, boost::algorithm::is_any_of("\t "), boost::token_compress_on);
		
		if (elements.size() >= 2) {
		
			int index = toInt(elements[0]);
		
			if (elements.size() == 2) {
				if (elements[1] == "loop") {
					IndexColor ic = palette.front();
					ic.index = index;
					palette.push_back(ic);
				}
			} else if (elements.size() == 4) {
				int r = toInt(elements[1]);
				int g = toInt(elements[2]);
				int b = toInt(elements[3]);
				
				palette.push_back(IndexColor(index, r, g, b));
			}
		}
	}

	std::string name = path.stem().string();
	std::ofstream os((outputPath / (name + ".h")).string());
	os << "#pragma once" << std::endl;
	os << "inline RGB " << name << "(uint16_t x) {" << std::endl;
	
	//os << "\tuint8_t r, g, b;" << std::endl;
	os << "\tRGB color;" << std::endl;

	subdivide(os, "\t", palette, 0, palette.size()-1);
	
	//os << "\tRGB color;" << std::endl;
	//os << "\tcolor.red = r;" << std::endl;
	//os << "\tcolor.green = g;" << std::endl;
	//os << "\tcolor.blue = b;" << std::endl;
	os << "\treturn color;" << std::endl;

	os << "}" << std::endl;
	os.close();
}



int main(int argc, const char **argv) {
	boost::filesystem::path outputPath = "generated";
	boost::filesystem::create_directory(outputPath);
	
	cos(outputPath);
	exp(outputPath);
	permute(outputPath);
	
	// convert palettes in current directory
	boost::filesystem::path p = ".";
	for (auto & entry : boost::make_iterator_range(boost::filesystem::directory_iterator(p), {})) {
		boost::filesystem::path path = entry.path();
		std::string ext = path.extension().string();
		if (ext == ".palette") {
			palette(path, outputPath);
		}
	}

	return 0;
}
