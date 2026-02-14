/*
* Copyright (C) 2019 - 2026 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#include "parameters.h"
#include "renderer.h"
#include "spectrogram.h"
#include "spectrum.h"
#include "tests.h"
#include "window.h"

#include <sndfile.hh>
#include <cstring>
#include <iostream>
#include <cmath>
#include <algorithm>

int main(int argc, char** argv)
{
	Sndspec::Parameters parameters;
	std::string msg(parameters.fromArgs({argv + 1, argv + argc}));
	if (!msg.empty()) {
		std::cout << msg << std::endl;
		exit(0);
	}

	if (parameters.getPlotWindowFunction()) {
		Sndspec::Spectrum::makeWindowFunctionPlot(parameters);
	} else if (parameters.getSpectrumMode()) {
		Sndspec::Spectrum::makeSpectrumFromFile(parameters);
	} else {
		Sndspec::Spectrogram::makeSpectrogramFromFile(parameters);
	}

	return 0;
}
