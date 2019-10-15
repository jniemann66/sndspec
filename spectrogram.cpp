#include "spectrogram.h"
#include "window.h"
#include "reader.h"
#include "spectrum.h"
#include "uglyplot.h"
#include "renderer.h"

#include <iostream>
#include <iomanip>

#include <cassert>

void Sndspec::Spectrogram::makeSpectrogram(const Sndspec::Parameters &parameters)
{
	static const int reservedChannels(2); // stereo (most common use case)
	auto fftSize = Spectrum::selectBestFFTSizeFromSpectrumSize(parameters.getImgHeight());
	auto spectrumSize = Spectrum::convertFFTSizeToSpectrumSize(fftSize);
	std::cout << "fft size " << fftSize << " spectrum size " << spectrumSize << std::endl;

	// make a suitable FFT Window
	Sndspec::KaiserWindow<double> k;
	k.setBeta(Sndspec::KaiserWindow<double>::betaFromDecibels(parameters.getDynRange()));
	k.generate(fftSize);

	// prepare a renderer
	Renderer renderer(parameters.getImgWidth(), spectrumSize);

	// prepare storage for spectrogram results
	SpectrogramResults<double> spectrogramData;
	spectrogramData.reserve(reservedChannels);

	// prepare the spectrum analyzers
	std::vector<std::unique_ptr<Spectrum>> analyzers;
	analyzers.reserve(reservedChannels);

	for(const std::string& inputFilename : parameters.getInputFiles()) {
		std::cout << "Opening input file: " << inputFilename << " ... ";
		Sndspec::Reader<double> r(inputFilename, fftSize, parameters.getImgWidth());

		if(r.getSndFileHandle() == nullptr || r.getSndFileHandle()->error() != SF_ERR_NO_ERROR) {
			std::cout << "couldn't open file !" << std::endl;
		} else {
			std::cout << "ok" << std::endl;
			int nChannels = r.getNChannels();
			std::cout << "channels: " << nChannels << std::endl;

			// provide the reader with the FFT window. The Reader will apply the window to each block it reads.
			r.setWindow(k.getData());

			// resize output storage (according to number of channels)
			spectrogramData.resize(nChannels, std::vector<std::vector<double>>(parameters.getImgWidth(), std::vector<double>(spectrumSize, 0.0)));

			for(int ch = 0; ch < nChannels; ch ++) {

				// create a spectrum analyzer for each channel if not already existing
				if(ch + 1 > analyzers.size()) {
					analyzers.emplace_back(new Spectrum(fftSize));
				}

				// give the reader direct write-access to the analyzer input buffer
				r.setChannelBuffer(ch, analyzers.at(ch)->getTdBuf());
			}

			// create a callback function to execute spectrum analysis for each block read
			r.setProcessingFunc([&analyzers, &spectrogramData](int pos, int channel, const double* data) -> void {
				Spectrum* analyzer = analyzers.at(channel).get();
				assert(data == analyzer->getTdBuf());
				analyzer->exec();
				analyzer->getMagSquared(spectrogramData[channel][pos]); // magSquared avoids having do to square root !
			});

			// read (and analyze) the file
			r.readDeinterleaved();

			// scale the data into dB
			scaleMagnitudeRelativeDb(spectrogramData, /* magSquared = */ true);

			std::cout << "Rendering ... ";
			renderer.Render(parameters, spectrogramData);
			std::cout << "Done\n";

			// save output file. todo : proper management of paths / filenames / extensions
			std::string outFile = replaceFileExt(inputFilename, "png");
			if(!outFile.empty()) {
				std::cout << "Saving to " << outFile << std::flush;
				if(renderer.writeToFile(outFile)) {
					std::cout << " ... OK" << std::endl;
				} else {
					std::cout << " ... ERROR" << std::endl;
				}
			}

		} // ends successful file-open
	} // ends loop over files
}

void Sndspec::Spectrogram::scaleMagnitudeRelativeDb(SpectrogramResults<double> &s, bool fromMagSquared)
{
	int numChannels = s.size();
	int numSpectrums = s.at(0).size();
	int numBins = s.at(0).at(0).size();

	std::vector<double> peak(numChannels, 0.0);

	for(int c = 0; c < numChannels; c++) {

		// find peak
		double peak{0.0};
		for(int x = 0; x < numSpectrums; x++) {
			for(int b = 0; b < numBins; b++) {
				peak = std::max(peak, s[c][x][b]);
			}
		}

		if(std::fpclassify(peak) != FP_ZERO) {

			// function to convert to dB
			auto scaleFunc = [scale = 1.0 / peak, dBMult = fromMagSquared ? 10.0 : 20.0] (double v) -> double {
				return dBMult * std::log10(scale * v);
			};

			// scale the data
			for(int x = 0; x < numSpectrums; x++) {
				std::transform (s[c][x].begin(), s[c][x].end(), s[c][x].begin(), scaleFunc);
			}
		}
	}
}

std::string Sndspec::Spectrogram::replaceFileExt(const std::string& filename, const std::string &newExt)
{

	auto lastDot = filename.rfind('.', filename.length());

	if(lastDot != std::string::npos) {
		std::string _fn{filename};
		_fn.replace(lastDot + 1, newExt.length(), newExt);
		return _fn;
	}

	return filename + "." + newExt;
}

