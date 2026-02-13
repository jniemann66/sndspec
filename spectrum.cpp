/*
* Copyright (C) 2019 - 2026 Judd Niemann - All Rights Reserved.
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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace Sndspec {

// todo: this is only good for doubles: specialize for FloatType
Spectrum::Spectrum(int fft_size)
	: fftSize(fft_size)
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

	for (int ef : {1, 11, 13}) {
		for (int d = 1; d <= requested_size; d *= 7) {
			for (int c = 1; c <= requested_size; c *= 5) {
				for (int b = 1; b <= requested_size; b *= 3) {
					for (int a = 1; a <= requested_size; a *= 2) {
						int t = a * b * c * d * ef;
						if (t > requested_size) {
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
	for (int b = 0; b < spectrumSize; b++) {
		double re = fdBuf[b][0];
		double im = fdBuf[b][1];
		buf[static_cast<std::vector<double>::size_type>(b)] = std::sqrt(re * re + im * im);
	}
}

void Spectrum::getMagSquared(std::vector<double>& buf)
{
	for (int b = 0; b < spectrumSize; b++) {
		double re = fdBuf[b][0];
		double im = fdBuf[b][1];
		buf[static_cast<std::vector<double>::size_type>(b)] = re * re + im * im;
	}
}

void Spectrum::getPhase(std::vector<double>& buf)
{
	for (int b = 0; b < spectrumSize; b++) {
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
	for (int c = 0; c < numChannels; c++) {
		for (int b = 0; b < numBins; b++) {
			peak = std::max(peak, s[c][b]);
		}
	}

	if (std::fpclassify(peak) != FP_ZERO) { // scale the data

		hasSignal = true;
		double dBMult = fromMagSquared ? 10.0 : 20.0;

		// set a floor to avoid log(0) problems
		double floor = std::max(std::numeric_limits<double>::min(), peak * pow(10.0, -300.0 / dBMult)); // 300dB below peak or smallest normal number

		// function to convert to dB
		auto scaleFunc = [scale = 1.0 / peak, dBMult, floor] (double v) -> double {
			return dBMult * std::log10(std::max(scale * v, floor));
		};

		for (int c = 0; c < numChannels; c++) {
			std::transform(s[c].begin(), s[c].end(), s[c].begin(), scaleFunc);
		}
	}

	return hasSignal;
}

bool Spectrum::convertToDb(std::vector<double> &s, bool fromMagSquared)
{
	int numChannels = s.size();
	bool hasSignal{false};
	int numBins = s.size();

	// find peak
	double peak{0.0};
	for (int b = 0; b < numBins; b++) {
		peak = std::max(peak, s[b]);
	}

	if (std::fpclassify(peak) != FP_ZERO) { // scale the data

		hasSignal = true;
		double dBMult = fromMagSquared ? 10.0 : 20.0;

		// set a floor to avoid log(0) problems
		double floor = std::max(std::numeric_limits<double>::min(), peak * pow(10.0, -300.0 / dBMult)); // 300dB below peak or smallest normal number

		// function to convert to dB
		auto scaleFunc = [scale = 1.0 / peak, dBMult, floor] (double v) -> double {
			return dBMult * std::log10(std::max(scale * v, floor));
		};

		std::transform(s.begin(), s.end(), s.begin(), scaleFunc);

	}

	return hasSignal;
}


bool Spectrum::convertToLinear(std::vector<std::vector<double>> &s, bool fromMagSquared)
{
	int numChannels = s.size();
	bool hasSignal{false};
	int numBins = s.at(0).size();

	for (int c = 0; c < numChannels; c++) {
		// find peak
		double peak{0.0};
		if (fromMagSquared) {
			for (int b = 0; b < numBins; b++) {
				peak = std::max(peak, std::sqrt (s[c][b]));
			}
		} else {
			for (int b = 0; b < numBins; b++) {
				peak = std::max(peak, s[c][b]);
			}
		}

		if (std::fpclassify(peak) != FP_ZERO) {
			hasSignal = true;
			const double scale = 100.0 / peak;

			// function to convert to percentage of fullScale
			if (fromMagSquared) {
				auto scaleFunc = [scale] (double v) -> double {
					return scale * std::sqrt(v) - 100.0;
				};

				// scale the data
				std::transform (s[c].begin(), s[c].end(), s[c].begin(), scaleFunc);

			} else {
				auto scaleFunc = [scale] (double v) -> double {
					return scale * v - 100.0;
				};

				// scale the data
				std::transform (s[c].begin(), s[c].end(), s[c].begin(), scaleFunc);
			}
		}
	}
	return hasSignal;
}

void Spectrum::makeSpectrumFromFile(const Sndspec::Parameters &parameters)
{
	// prepare a renderer
	Renderer renderer(parameters.getImgWidth(), parameters.getImgHeight());

	// loop over the files
	for (const std::string& inputFilename : parameters.getInputFiles()) {
		int nChannels;
		int sampleRate;

		// open file
		std::cout << "Opening input file: " << inputFilename << " ... ";
		Sndspec::Reader<double> r(inputFilename, 0, 1);
		if (r.getSndFileHandle() == nullptr || r.getSndFileHandle()->error() != SF_ERR_NO_ERROR) {
			std::cout << "couldn't open file !" << std::endl;
			return;
		} else {
			std::cout << "ok" << std::endl;
			nChannels = r.getNChannels();
			sampleRate = r.getSamplerate();
			renderer.setNyquist(sampleRate / 2);
			renderer.setFreqStep(parameters.getFrequencyStep());
		}

		// calculate blocksize
		int64_t startPos  = std::max(0, std::min(static_cast<int>(r.getSamplerate() * parameters.getStart()),  r.getNFrames()));
		int64_t finishPos = (parameters.getFinish() == 0) ?
			r.getNFrames() :
			std::max(0, std::min(static_cast<int>(r.getSamplerate() * parameters.getFinish()), r.getNFrames()));
		int interval = static_cast<int>(std::max(INT64_C(0), finishPos - startPos));
		int blockSize = Spectrum::selectBestFFTSize(interval);
		assert(blockSize <= interval);
		r.setStartPos(startPos);
		r.setFinishPos(startPos + blockSize);
		r.setBlockSize(blockSize);

		// create window
		Sndspec::Window<double> window;
		const double param
				= parameters.getWindowFunctionParameters().empty() ?
					   Sndspec::Window<double>::kaiserBetaFromDecibels(parameters.getDynRange())
					 : parameters.getWindowFunctionParameters().at(0);
		window.generate(parameters.getWindowFunction(), blockSize, param);

		r.setWindow(window.getData());

		// prepare the spectrum analyzers
		std::vector<std::unique_ptr<Spectrum>> analyzers;
		for (int ch = 0; ch < nChannels; ch ++) {
			// create a spectrum analyzer for each channel if not already existing
			analyzers.emplace_back(new Spectrum(blockSize));
			r.setChannelBuffer(ch, analyzers.at(ch)->getTdBuf()); // give the reader direct write-access to the analyzer input buffer
		}

		// set a callback function to execute spectrum analysis for each block read
		r.setProcessingFunc([&analyzers](int pos, int channel, const double* data) -> void {
			(void)pos;
			(void)data;
			analyzers.at(channel)->exec();
		});

		// read (and analyze) the file
		if (parameters.getChannelMode() == Sum) {
			r.readSum();
		} else if (parameters.getChannelMode() == Difference) {
			r.readDifference();
		} else {
			r.readDeinterleaved();
		}

		// prepare and populate results buffers
		std::vector<std::vector<double>> results;
		for (int ch = 0; ch < nChannels; ch ++) {
			results.emplace_back(analyzers.at(ch)->getSpectrumSize(), 0.0);
			analyzers.at(ch)->getMagSquared(results.at(ch));
		}

		bool hasSignal = false;
		if (parameters.getLinearMag()) {
			hasSignal = Spectrum::convertToLinear(results, /* fromMagSquared = */ true);
		} else {
			// scale to dB
			hasSignal = Spectrum::convertToDb(results, /* fromMagSquared = */ true);
		}

		// determine which channels to plot
		if (parameters.getChannelMode() == Sum || parameters.getChannelMode() == Difference) {
			std::vector<bool> enabled(nChannels, false);
			enabled[0] = true; //hasSignal;
			renderer.setChannelsEnabled(enabled);
		} else {
			renderer.setChannelsEnabled(std::vector<bool>(nChannels, hasSignal));
		}

		renderer.setInputFilename(inputFilename);
		renderer.setDynRange(parameters.getDynRange());
		renderer.setTitle("Spectrum");
		renderer.setHorizAxisLabel("Frequency (Hz)");

		if (parameters.getLinearMag()) {
			renderer.setVertAxisLabel("Relative Magnitude (%)");
		} else {
			renderer.setVertAxisLabel("Relative Magnitude (dB)");
		}

		double startTime = static_cast<double>(r.getStartPos()) / r.getSamplerate();
		double finishTime = static_cast<double>(r.getFinishPos()) / r.getSamplerate();
		renderer.setStartTime(startTime);
		renderer.setFinishTime(finishTime);
		const auto& [magnitudes, vScaling] = renderer.renderSpectrum(parameters, results);

		if (parameters.getTopN().has_value()) {
			const size_t n = parameters.getTopN().value();
			const std::vector<bool> enabled = renderer.getChannelsEnabled();
			for (int ch = 0; ch < nChannels; ch++) {
				if (enabled[ch]) {
					const auto markers = renderer.getTopNFrequencyMarkers(parameters, magnitudes.at(ch), vScaling, n, ch);
					// todo: if desired, do something else with markers (summarize in table, send to stdout etc ...)
					renderer.drawMarkers(markers);
				}
			}
		}

		if (parameters.hasWhiteBackground()) {
			renderer.makeNegativeImage();
		}

		// determine output filename
		std::string outputFilename;
		if (parameters.getOutputPath().empty()) {
			outputFilename = replaceFileExt(inputFilename, "png");
		} else {
			outputFilename = enforceTrailingSeparator(parameters.getOutputPath()) + getFilenameOnly(replaceFileExt(inputFilename, "png"));
		}

		if (!outputFilename.empty()) {
			std::cout << "Saving to " << outputFilename << std::flush;
			if (renderer.writeToFile(outputFilename)) {
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

void Spectrum::makeWindowFunctionPlot(const Parameters &parameters)
{
	const int w = parameters.getImgWidth();
	const int h = parameters.getImgHeight();

	// generate a window with this many sample points
	constexpr size_t windowSize = 256;

	// for smooth spectrum results, make the fft Size considerably larger than the actual window (pad with zeroes)
	constexpr int fftSizeRato = 16;
	constexpr size_t fftSize = fftSizeRato * windowSize;

	// generate the window
	Sndspec::Window<double> window;
	const double param
			= parameters.getWindowFunctionParameters().empty() ?
				   Sndspec::Window<double>::kaiserBetaFromDecibels(parameters.getDynRange())
				 : parameters.getWindowFunctionParameters().at(0);
	window.generate(parameters.getWindowFunction(), windowSize, param);

	if (parameters.getPlotTimeDomain()) {
		std::string name = parameters.getWindowFunctionDisplayName() + "-time_domain";
		std::replace(name.begin(), name.end(), ' ', '_');

		// plot the window
		std::cout << "w=" << w << " h=" << h << " window size=" << window.getData().size() << std::endl;
		Sndspec::Renderer r{w, h};
		r.setNyquist(windowSize);
		r.setFreqStep(windowSize / 10);
		r.setInputFilename(name);
		r.setDynRange(parameters.getDynRange());
		r.setTitle("Window Function");
		r.setHorizAxisLabel("Time (sample index)");

		if (parameters.getLinearMag()) {
			r.setVertAxisLabel("Relative Magnitude (%)");
		} else {
			r.setVertAxisLabel("Relative Magnitude (dB)");
		}

		r.renderWindowFunction(parameters, window.getData());

		if (parameters.hasWhiteBackground()) {
			r.makeNegativeImage();
		}

		r.writeToFile(name + ".png");
	} else {
		std::string name = parameters.getWindowFunctionDisplayName() + "-freq_domain";
		std::replace(name.begin(), name.end(), ' ', '_');

		// horizontal scaling factor. Values > 1 allow zooming-in to see more detail around f=0
		const double horizontalZoom = parameters.getHorizZoomFactor();

		// make a copy of the window data, and resize it to fftSize by padding with zeroes
		std::vector<double> wd = window.getData();
		std::cout << "fft size=" << fftSize << std::endl;
		wd.resize(fftSize, 0.0);

		// initialise Spectrum object, and copy window data to it's time-domain buffer
		Sndspec::Spectrum s(fftSize);
		std::memcpy(s.getTdBuf(), wd.data(), fftSize * sizeof(double));
		s.exec(); // run the fft

		// get magnitude spectrum
		std::vector<double> v(fftSize, 0.0);
		s.getMag(v);
		s.convertToDb(v, false);

		// code to calculate width at -3dB
		// std::cout << parameters.getWindowFunction() << " ";
		// for (size_t i = 0; i < v.size(); i++) {
		// 	double d = v.at(i);
		// 	if (d <= -3.0) {
		// 		std::cout << "-3dB width =" << 2 * i << std::endl;
		// 		break;
		// 	}
		// }

		if (horizontalZoom > 1) {
			// zoom accomplished by simply shrinking results to area of interest
			v.resize(v.size() / horizontalZoom);
		}

		Sndspec::Renderer r{w, h};
		r.setNyquist(100.0);
		r.setFreqAxisFormat(Renderer::FreqAxisFormat_PlusMinusNormalisedFreq);
		r.setFreqStep(10);
		r.setHorizZoomFactor(horizontalZoom);
		r.setInputFilename(name);
		r.setDynRange(parameters.getDynRange());
		r.setTitle("Window Function");
		r.setHorizAxisLabel("Normalised Frequency (x π radians / sample)");


		if (parameters.getLinearMag()) {
			r.setVertAxisLabel("Relative Magnitude (%)");
		} else {
			r.setVertAxisLabel("Relative Magnitude (dB)");
		}
		r.renderWindowFunction(parameters, v);

		if (parameters.hasWhiteBackground()) {
			r.makeNegativeImage();
		}

		r.writeToFile(name + ".png");
	}
}

std::map<double, size_t, std::greater<double>> Spectrum::getRankedLocalMaxima(const std::vector<double>& data)
{
	std::map<double, size_t, std::greater<double>> results; // retval
	if (data.size() >= 3) { // need at least 3 elements
		// Iterate over all elements except first and last
		for (size_t i = 1; i < data.size() - 1; ++i) {
			if (data[i] > data[i - 1] && data[i] > data[i + 1]) {
				// Found a local maximum
				results[data[i]] = i;
			}
		}
	}

	return results;
}

} // namespace Sndspec
