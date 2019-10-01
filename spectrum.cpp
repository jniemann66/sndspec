#include "spectrum.h"

#include <vector>
#include <cmath>

namespace Sndspec {

// todo: this is only good for doubles: specialize for FloatType
Spectrum::Spectrum(int fftLength) : fftLength(fftLength)
{
	tdBuf = static_cast<double*>(fftw_malloc(sizeof(double) * static_cast<size_t>(fftLength)));
	fdBuf = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * static_cast<size_t>(fftLength)));
	mag.resize(static_cast<std::vector<double>::size_type>(fftLength));
	phase.resize(static_cast<std::vector<double>::size_type>(fftLength));
}

Spectrum::~Spectrum()
{
	fftw_free(tdBuf);
	fftw_free(fdBuf);
}

double *Spectrum::getTdBuf() const
{
	return tdBuf;
}

const fftw_complex *Spectrum::getFdBuf() const
{
	return fdBuf;
}

std::vector<double> Spectrum::getMag()
{
	for(int b = 0; b < fftLength; b++) {
		double re = fdBuf[b][0];
		double im = fdBuf[b][1];
		mag[static_cast<std::vector<double>::size_type>(b)] = std::sqrt(re * re + im * im);
	}
	return mag;
}

std::vector<double> Spectrum::getPhase()
{
	for(int b = 0; b < fftLength; b++) {
		double re = fdBuf[b][0];
		double im = fdBuf[b][1];
		phase[static_cast<std::vector<double>::size_type>(b)] = std::atan2(im, re);
	}
	return phase;
}



// todo: float version
// todo: long double version
// todo: quad precision version etc


} // namespace Sndspec
