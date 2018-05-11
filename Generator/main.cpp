#include <fstream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>
#include "hsv2rgb.h"


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

int main(int argc, const char **argv) {
	boost::filesystem::path outputPath = "generated";
	boost::filesystem::create_directory(outputPath);
	
	exp(outputPath);
	permute(outputPath);
	hsv2rgb(outputPath);
	
	// convert palettes in current directory
	boost::filesystem::path p = ".";
	for (auto & entry : boost::make_iterator_range(boost::filesystem::directory_iterator(p), {})) {
		boost::filesystem::path path = entry.path();
		std::string ext = path.extension().string();
		if (ext == ".palette") {
			//path.replace_extension(".palette.h");
			boost::filesystem::path name = path.filename();
			name.replace_extension(".palette.h");
			std::ofstream s((outputPath / name).string());
			
			s.close();
		}
	}

	return 0;
}
