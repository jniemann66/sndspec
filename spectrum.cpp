/*
* Copyright (C) 2016 - 2019 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#include "spectrum.h"
#include "renderer.h"
#include "reader.h"
#include "window.h"

#include <vector>
#include <cmath>
#include <algorithm>

#include <iostream>

// todo: float version
// todo: long double version
// todo: quad precision version etc


namespace Sndspec {

// todo: this is only good for doubles: specialize for FloatType
Spectrum::Spectrum(int fft_size) : fftSize(fft_size)
{
	spectrumSize = convertFFTSizeToSpectrumSize(fftSize);

	tdBuf = static_cast<double*>(fftw_malloc(sizeof(double) * static_cast<size_t>(fftSize)));
	fdBuf = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * static_cast<size_t>(fftSize)));
	plan = fftw_plan_dft_r2c_1d(fftSize, tdBuf, fdBuf, FFTW_MEASURE | FFTW_PRESERVE_INPUT);
}

Spectrum::~Spectrum()
{
	fftw_destroy_plan(plan);
	fftw_free(tdBuf);
	fftw_free(fdBuf);
}

void Spectrum::exec()
{	
	fftw_execute(plan);
}

int Spectrum::getFFTSize() const
{
	return fftSize;
}

int Spectrum::getSpectrumSize() const
{
	return spectrumSize;
}

int Spectrum::selectBestFFTSizeFromSpectrumSize(int spectrum_size)
{
	return selectBestFFTSize(convertSpectrumSizeToFFTSize(spectrum_size));
}

int Spectrum::selectBestFFTSize(int requested_size)
{
	int s = 1;

	for(int ef : {1, 11, 13}) {
		for(int d = 1; d <= requested_size; d *= 7) {
			for(int c = 1; c <= requested_size; c *= 5) {
				for(int b = 1; b <= requested_size; b *= 3) {
					for(int a = 1; a <= requested_size; a *= 2) {
						int t = a * b * c * d * ef;
						if(t > requested_size) {
							break;
						}
						s = std::max(s, t);
					}
				}
			}
		}
	}

	return s;
}

int Spectrum::convertSpectrumSizeToFFTSize(int spectrum_size)
{
	return 2 * (spectrum_size - 1);
}

int Spectrum::convertFFTSizeToSpectrumSize(int fft_size)
{
	return static_cast<int>(fft_size / 2.0) + 1;
}


double* Spectrum::getTdBuf() const
{
	return tdBuf;
}

const fftw_complex *Spectrum::getFdBuf() const
{
	return fdBuf;
}

void Spectrum::getMag(std::vector<double>& buf)
{
	for(int b = 0; b < spectrumSize; b++) {
		double re = fdBuf[b][0];
		double im = fdBuf[b][1];
		buf[static_cast<std::vector<double>::size_type>(b)] = std::sqrt(re * re + im * im);
	}
}

void Spectrum::getMagSquared(std::vector<double>& buf)
{
	for(int b = 0; b < spectrumSize; b++) {
		double re = fdBuf[b][0];
		double im = fdBuf[b][1];
		buf[static_cast<std::vector<double>::size_type>(b)] = re * re + im * im;
	}
}

void Spectrum::getPhase(std::vector<double>& buf)
{
	for(int b = 0; b < spectrumSize; b++) {
		double re = fdBuf[b][0];
		double im = fdBuf[b][1];
		buf[static_cast<std::vector<double>::size_type>(b)] = std::atan2(im, re);
	}
}

bool Spectrum::convertToDb(std::vector<std::vector<double>> &s, bool fromMagSquared)
{
	int numChannels = s.size();
	bool hasSignal{false};
	int numBins = s.at(0).size();

	// find peak
	double peak{0.0};
	for(int c = 0; c < numChannels; c++) {
		for(int b = 0; b < numBins; b++) {
			peak = std::max(peak, s[c][b]);
		}
	}

	if(std::fpclassify(peak) != FP_ZERO) { // scale the data

		hasSignal = true;
		double dBMult = fromMagSquared ? 10.0 : 20.0;

		// set a floor to avoid log(0) problems
		double floor = std::max(std::numeric_limits<double>::min(), peak * pow(10.0, -300.0 / dBMult)); // 300dB below peak or smallest normal number

		// function to convert to dB
		auto scaleFunc = [scale = 1.0 / peak, dBMult, floor] (double v) -> double {
			return dBMult * std::log10(std::max(scale * v, floor));
		};

		for(int c = 0; c < numChannels; c++) {
			std::transform (s[c].begin(), s[c].end(), s[c].begin(), scaleFunc);
		}
	}

	return hasSignal;
}

void Spectrum::makeSpectrumFromFile(const Sndspec::Parameters &parameters)
{
	// prepare a renderer
	Renderer renderer(parameters.getImgWidth(), parameters.getImgHeight());

	// loop over the files
	for(const std::string& inputFilename : parameters.getInputFiles()) {
		int nChannels;
		int sampleRate;

		// open file
		std::cout << "Opening input file: " << inputFilename << " ... ";
		Sndspec::Reader<double> r(inputFilename, 0, 1);
		if(r.getSndFileHandle() == nullptr || r.getSndFileHandle()->error() != SF_ERR_NO_ERROR) {
			std::cout << "couldn't open file !" << std::endl;
		} else {
			std::cout << "ok" << std::endl;
			nChannels = r.getNChannels();
			sampleRate = r.getSamplerate();
			renderer.setNyquist(sampleRate / 2);
			renderer.setFreqStep(5000);
		}

		// calculate blocksize
		r.setStartPos(std::max(0, std::min(static_cast<int>(r.getSamplerate() * parameters.getStart()), r.getNFrames())));
		int64_t finishPos = std::max(0, std::min(static_cast<int>(r.getSamplerate() * parameters.getFinish()), r.getNFrames()));
		int interval = static_cast<int>(std::max(INT64_C(0), finishPos - r.getStartPos()));
		int blockSize = Spectrum::selectBestFFTSize(interval);
		r.setFinishPos(r.getStartPos() + blockSize);
		r.setBlockSize(blockSize);

		// create window
		Sndspec::Window<double> window;
		window.generate(parameters.getWindowFunction(), blockSize, Sndspec::Window<double>::kaiserBetaFromDecibels(parameters.getDynRange()));
		r.setWindow(window.getData());

		// prepare the spectrum analyzers
		std::vector<std::unique_ptr<Spectrum>> analyzers;
		for(int ch = 0; ch < nChannels; ch ++) {
			// create a spectrum analyzer for each channel if not already existing
			analyzers.emplace_back(new Spectrum(blockSize));
			r.setChannelBuffer(ch, analyzers.at(ch)->getTdBuf()); // give the reader direct write-access to the analyzer input buffer
		}

		// set a callback function to execute spectrum analysis for each block read
		r.setProcessingFunc([&analyzers](int pos, int channel, const double* data) -> void {
			analyzers.at(channel)->exec();
		});

		// read (and analyze) the file
		r.readDeinterleaved();

		// prepare and populate results buffers
		std::vector<std::vector<double>> results;
		for(int ch = 0; ch < nChannels; ch ++) {
			results.emplace_back(analyzers.at(ch)->getSpectrumSize(), 0.0);
			analyzers.at(ch)->getMagSquared(results.at(ch));
		}

		// scale to dB
		bool hasSignal = Spectrum::convertToDb(results, true);

		// render
		if(!hasSignal) { // no signal
			renderer.setChannelsEnabled(std::vector(nChannels, false)); // don't plot anything
		}
		renderer.setInputFilename(inputFilename);
		renderer.setDynRange(parameters.getDynRange());
		renderer.setTitle("Spectrum");
		renderer.setHorizAxisLabel("Frequency (Hz)");
		renderer.setVertAxisLabel("Relative Magnitude (dB)");
		renderer.renderSpectrum(parameters, results);

		if(parameters.hasWhiteBackground()) {
			renderer.makeNegativeImage();
		}

		// determine output filename
		std::string outputFilename;
		if(parameters.getOutputPath().empty()) {
			outputFilename = replaceFileExt(inputFilename, "png");
		} else {
			outputFilename = enforceTrailingSeparator(parameters.getOutputPath()) + getFilenameOnly(replaceFileExt(inputFilename, "png"));
		}

		if(!outputFilename.empty()) {
			std::cout << "Saving to " << outputFilename << std::flush;
			if(renderer.writeToFile(outputFilename)) {
				std::cout << " ... OK" << std::endl;
			} else {
				std::cout << " ... ERROR" << std::endl;
			}
		} else {
			std::cout << "Error: couldn't deduce output filename" << std::endl;
		}

		renderer.clear();
	}
}

} // namespace Sndspec
