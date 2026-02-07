/*
* Copyright (C) 2019 - 2026 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#include "spectrogram.h"
#include "spectrum.h"
#include "parameters.h"
#include "window.h"
#include "tests.h"

#include <sndfile.hh>

#include <iostream>
#include <cmath>

int main(int argc, char** argv)
{
	Sndspec::Parameters parameters;
	std::string msg(parameters.fromArgs({argv + 1, argv + argc}));
	if (!msg.empty()) {
		std::cout << msg << std::endl;
		exit(0);
	}

	if (parameters.getPlotWindowFunction()) {
		// make sample size next power-of-2 >= width
		const size_t size = 1 << (1 + static_cast<int>(std::log2(parameters.getImgWidth())));
		Sndspec::Window<double> window;
		window.generate(parameters.getWindowFunction(), size, Sndspec::Window<double>::kaiserBetaFromDecibels(parameters.getDynRange()));
		if (parameters.getPlotTimeDomain()) {
			// todo: plot the window
		} else {
			// todo: plot fft magnitude of the window
		}
	} else if (parameters.getSpectrumMode()) {
		Sndspec::Spectrum::makeSpectrumFromFile(parameters);
	} else {
		Sndspec::Spectrogram::makeSpectrogramFromFile(parameters);
	}

	return 0;
}
