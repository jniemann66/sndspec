/*
* Copyright (C) 2019 - 2026 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/



#include "tests.h"
#include "parameters.h"
#include "window.h"
#include "spectrum.h"

bool tests::testWindow()
{
	Sndspec::Window<double> w;
	w.generate("triangular", 9, 20);
	for (double v : w.getData())
	{
		std::cout << v << std::endl;
	}
    return true;
}

bool tests::testAllWindows()
{
	Sndspec::Parameters p;
	p.setPlotWindowFunction(true);
	p.setImgWidth(1280);
	p.setIngHeight(960);
	for (const auto& wd : Sndspec::windowDefinitions) {
		p.setWindowFunction(wd.name);
		p.setWindowFunctionDisplayName(wd.displayName);
		Sndspec::Spectrum::makeWindowFunctionPlot(p);
	}
	return true;
}

bool tests::testMinus3dbWidth()
{
	for (const auto& wd : Sndspec::windowDefinitions) {
		std::cout << wd.displayName << ": " << Sndspec::Spectrum::getMinus3dbWidth(wd.name, {} ) << "\n";
	}
	std::cout << std::endl;
	return true;
}
