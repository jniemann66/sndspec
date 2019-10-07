#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <fftw3.h>

#include <vector>

namespace Sndspec {

class Spectrum
{
public:
	Spectrum(int fftLength);
	~Spectrum();

	double* getTdBuf() const;
	const fftw_complex *getFdBuf() const;
	void getMag(std::vector<double>& buf);
	void getMagSquared(std::vector<double> &buf);
	void getPhase(std::vector<double>& buf);
	void exec();

	int getFftLength() const;
	int getSpectrumLength() const;
//	std::pair<double, int> getFdPeak() const;

private:
	fftw_plan plan;
	int fftLength;
	int spectrumLength;

	// C facing:
	double* tdBuf;	// time-domain buffer
	fftw_complex* fdBuf; // frequency-domain buffer

};

} // namespace Sndspec

#endif // SPECTRUM_H
