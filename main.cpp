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
		static const int w = parameters.getImgWidth();
		static const int h = parameters.getImgHeight();
		// make sample size next power-of-2 >= width
		const size_t size = 1 << (static_cast<int>(std::log2(w - 1)) + 1);
		Sndspec::Window<double> window;
		window.generate(parameters.getWindowFunction(), size, Sndspec::Window<double>::kaiserBetaFromDecibels(parameters.getDynRange()));
		if (parameters.getPlotTimeDomain()) {
			// plot the window
			std::cout << "w=" << w << " h=" << h << " window size=" << window.getData().size() << std::endl;
			Sndspec::Renderer r{w, h};
			r.setNyquist(size * 2);
			r.setFreqStep(size / 10);
			r.setInputFilename(parameters.getWindowFunctionDisplayName() + " - time domain");
			r.setDynRange(parameters.getDynRange());
			r.setTitle("Window Function");
			r.setHorizAxisLabel("x");

			if (parameters.getLinearMag()) {
				r.setVertAxisLabel("Relative Magnitude (%)");
			} else {
				r.setVertAxisLabel("Relative Magnitude (dB)");
			}

			r.renderWindowFunction(parameters, window.getData());
			r.writeToFile("windowfunction.png");
		} else {
			Sndspec::Spectrum s(size);
			std::memcpy(s.getTdBuf(), window.getData().data(), size * sizeof(double));

			size_t fftSize = s.getFFTSize();
			std::cout << "fft size=" << fftSize << std::endl;
			s.exec();

			Sndspec::Renderer r{w, h};
			r.setNyquist(size);
			r.setFreqStep(size / 10);
			r.setInputFilename(parameters.getWindowFunctionDisplayName() + " - time domain");
			r.setDynRange(parameters.getDynRange());
			r.setTitle("Window Function");
			r.setHorizAxisLabel("x");

			if (parameters.getLinearMag()) {
				r.setVertAxisLabel("Relative Magnitude (%)");
			} else {
				r.setVertAxisLabel("Relative Magnitude (dB)");
			}

			std::vector<double> v(fftSize, 0.0);
			s.getMag(v);
			auto mm = std::minmax_element(v.begin(), v.end());
			s.convertToDb(v, false);
			std::cout << "min=" << *mm.first << " max=" << *mm.second << std::endl;
			r.renderWindowFunction(parameters, v);
			r.writeToFile("windowfunction.png");
		}
	} else if (parameters.getSpectrumMode()) {
		Sndspec::Spectrum::makeSpectrumFromFile(parameters);
	} else {
		Sndspec::Spectrogram::makeSpectrogramFromFile(parameters);
	}

	return 0;
}
