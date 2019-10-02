#ifndef UGLYPLOT_H
#define UGLYPLOT_H

#include <iostream>
#include <cmath>

class UglyPlot
{
public:
	static void plot(const double* p, int length)
	{
		constexpr int cols = 80;
		constexpr int center = cols / 2;
		double peak = 0.0;
		for(int n = 0; n < length; n++) {
			peak = std::max(std::abs(p[n]), peak);
		}
		double scale = center / peak;
		for(int r = 0; r < length; r++) {
			int a = std::round(center + scale * p[r]);
			int b = cols - a;
			std::string s(a, ' ');
			std::string t(b, ' ');
			std::cout << s << "*" << t << p[r] << "\n";
		}
		std::cout << std::endl;
	}
};

#endif // UGLYPLOT_H
