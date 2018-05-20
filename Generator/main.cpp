#include <fstream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>


void exp(boost::filesystem::path outputPath) {
	std::ofstream f((outputPath / "exp16_5.h").string());
	f << "#pragma once" << std::endl;
	
	// write exp table with 5 bit input and 16 bit output
	f << "TABLE16(exp16Table) = {" << std::endl;
	f << "\t0";
	for (int i = 1; i < 32; ++i) {
		//int y = int(round(pow(1.16, i) * 256.0));
		// x^31 = 65535 -> x = 65535^(1/31)
		int y = int(round(pow(65535.0, i / 31.0)));
		f << ", " << y;
	}
	f << std::endl;
	f << "};" << std::endl;
	f << "INLINE uint16_t exp16_5(uint8_t x) {return READ16(exp16Table, x);}" << std::endl;
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
	f << "TABLE8(permute8Table) = {" << std::endl;
	f << "\t";
	for (int i = 0; i < 256; ++i) {
		if (i != 0)
			f << ", ";
		f << permute[i];
	}
	f << std::endl;
	f << "};" << std::endl;
	f << "INLINE uint8_t permute8(uint8_t x) {return READ8(permute8Table, x);}" << std::endl;
}


struct IndexColor {
	int index;
	
	int r;
	int g;
	int b;

	IndexColor() = default;
	IndexColor(int index, uint8_t r, uint8_t g, uint8_t b) : index(index), r(r), g(g), b(b) {}
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
			f << "uint8_t(x)";
		else
			f << "uint8_t(x - " << index1 << ")";
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
		os << indent << "}" << std::endl;
		subdivide(os, indent, palette, mid, end);
	} else {
		IndexColor color1 = palette[begin];
		IndexColor color2 = palette[begin + 1];
		//f << indent << "// " << index << ": " << int(color1.r) << " " << int(color1.g) << " " << int(color1.b) << std::endl;
		
		os << indent << "return RGB(";
		interpolate(os, indent, color1.index, color2.index, color1.r, color2.r); os << ", ";
		interpolate(os, indent, color1.index, color2.index, color1.g, color2.g); os << ", ";
		interpolate(os, indent, color1.index, color2.index, color1.b, color2.b);
		os << ");" << std::endl;
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
	os << "INLINE RGB " << name << "(uint16_t x) {" << std::endl;
	subdivide(os, "\t", palette, 0, palette.size()-1);
	os << "}" << std::endl;
	os.close();
}



int main(int argc, const char **argv) {
	boost::filesystem::path outputPath = "generated";
	boost::filesystem::create_directory(outputPath);
	
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
