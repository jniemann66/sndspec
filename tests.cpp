#include "tests.h"
#include "window.h"

#include "uglyplot.h"

bool tests::testUglyPlot()
{
	double t[80];
	for(int x = 0; x < 80; x++) {
		t[x] = 0.001 * (x - 40);
	}
	UglyPlot::plot(t, 80);

	return true;
}

bool tests::testWindow()
{
	Sndspec::Window<double> w;
	w.generateBlackman(8);
	for(double v : w.getData())
	{
		std::cout << v << std::endl;
	}
}
