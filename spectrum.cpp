#include "spectrum.h"

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

void Spectrum::scaleMagnitudeRelativeDb(std::vector<std::vector<double>> &s, bool fromMagSquared)
{
	int numChannels = s.size();
	int numBins = s.at(0).size();

	std::vector<double> peak(numChannels, 0.0);

	for(int c = 0; c < numChannels; c++) {

		// find peak
		double peak{0.0};
		for(int x = 0; x < numChannels; x++) {
			for(int b = 0; b < numBins; b++) {
				peak = std::max(peak, s[c][b]);
			}
		}

		if(std::fpclassify(peak) != FP_ZERO) {

			// function to convert to dB
			auto scaleFunc = [scale = 1.0 / peak, dBMult = fromMagSquared ? 10.0 : 20.0] (double v) -> double {
				return dBMult * std::log10(scale * v);
			};

			// scale the data
			for(int x = 0; x < numChannels; x++) {
				std::transform (s[x].begin(), s[x].end(), s[x].begin(), scaleFunc);
			}
		}
	}
}


} // namespace Sndspec
