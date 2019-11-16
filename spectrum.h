#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <fftw3.h>

#include <vector>

#include "parameters.h"

namespace Sndspec {

static std::string replaceFileExt(const std::string &filename, const std::string &newExt);
static std::string getFilenameOnly(const std::string &path);
static std::string enforceTrailingSeparator(const std::string &directory);

class Spectrum
{
public:
	Spectrum(int fft_size);
	~Spectrum();

	double* getTdBuf() const;
	const fftw_complex *getFdBuf() const;
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

std::string replaceFileExt(const std::string& filename, const std::string &newExt)
{

	auto lastDot = filename.rfind('.', filename.length());

	if(lastDot != std::string::npos) {
		std::string _fn{filename};
		_fn.replace(lastDot + 1, std::string::npos, newExt);
		return _fn;
	}

	return filename + "." + newExt;
}

std::string getFilenameOnly(const std::string& path)
{
	static const char universalPathSeparator{'/'};
	auto lastSep = path.rfind(universalPathSeparator, path.length());

#ifdef _WIN32
	static const char nativePathSeparator{'\\'};
	auto lastNSep = path.rfind(nativePathSeparator, path.length());
	if(lastNSep != std::string::npos) {
		if(lastSep == std::string::npos) {
			lastSep = lastNSep;
		} else {
			lastSep = std::max(lastSep, lastNSep);
		}
	}
#endif

	if(lastSep == path.length() - 1) { // ends in separator; cannot be a file
		return {};
	} else if(lastSep == std::string::npos) { // no separator; path is already just a filename
		return path;
	}

	return path.substr(lastSep + 1);
}

std::string enforceTrailingSeparator(const std::string& directory)
{

	static const char universalPathSeparator{'/'};
	auto lastSep = directory.rfind(universalPathSeparator, directory.length());

#ifdef _WIN32
	static const char nativePathSeparator{'\\'};
	auto lastNSep = directory.rfind(nativePathSeparator, directory.length());
	if(lastNSep != std::string::npos) {
		if(lastSep == std::string::npos) {
			lastSep = lastNSep;
		} else {
			lastSep = std::max(lastSep, lastNSep);
		}
	}
#else
	static const char nativePathSeparator{'/'};
#endif

	if(lastSep == directory.length() - 1) { // ends in separator; good to go ...
		return directory;
	}

	// need to append a separator, but what kind ?
	if(nativePathSeparator != universalPathSeparator && directory.find(universalPathSeparator, directory.length()) != std::string::npos) {
		return directory + universalPathSeparator;
	}

	return directory + nativePathSeparator;
}


} // namespace Sndspec

#endif // SPECTRUM_H
