#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <fftw3.h>

#include <vector>

#include "parameters.h"

namespace Sndspec {

class Spectrum
{
public:
	Spectrum(int fft_size);
	~Spectrum();

	double* getTdBuf() const;
	const fftw_complex *getFdBuf() const;
	template <typename FloatType>
	static void makeSpectrumFromFile(const Sndspec::Parameters &parameters);
	void getMag(std::vector<double>& buf);
	void getMagSquared(std::vector<double> &buf);
	void getPhase(std::vector<double>& buf);
	void exec();

	int getFFTSize() const;
	int getSpectrumSize() const;

	static int convertSpectrumSizeToFFTSize(int spectrum_size);
	static int convertFFTSizeToSpectrumSize(int fft_size);
	static int selectBestFFTSizeFromSpectrumSize(int spectrum_size);
	static int selectBestFFTSize(int requested_size); // pick a good FFT size for FFTW (of the form 2^a * 3^b * 5^c * 7^d * [1|11|13] )
	static void scaleMagnitudeRelativeDb(std::vector<std::vector<double> > &s, bool fromMagSquared);


private:
	fftw_plan plan;
	int fftSize;
	int spectrumSize;

	// C facing:
	double* tdBuf;	// time-domain buffer
	fftw_complex* fdBuf; // frequency-domain buffer
};

} // namespace Sndspec

#endif // SPECTRUM_H
