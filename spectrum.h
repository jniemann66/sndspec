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

	double *getTdBuf() const;
	const fftw_complex *getFdBuf() const;
	std::vector<double> getMag();
	std::vector<double> getPhase();
	void exec();

	int getFftLength() const;
	int getSpectrumLength() const;
	std::pair<double, int> getFdPeak() const;

private:
	fftw_plan plan;
	int fftLength;
	int spectrumLength;

	// C facing:
	double* tdBuf;
	fftw_complex* fdBuf;

	// C++ facing:
	std::vector<double> mag;
	std::vector<double> phase;
};

} // namespace Sndspec

#endif // SPECTRUM_H
