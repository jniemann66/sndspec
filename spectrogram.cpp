#include "spectrogram.h"
#include "window.h"
#include "reader.h"
#include "spectrum.h"
#include "uglyplot.h"

#include <iostream>
#include <cassert>

void Sndspec::Spectrogram::makeSpectrogram(const Sndspec::Parameters &parameters)
{
	auto fftSize = parameters.getFftSize();
	auto spectrumSize = fftSize >> 1;

	// make a suitable Kaiser window
	Sndspec::KaiserWindow<double> k;
	k.setBeta(Sndspec::KaiserWindow<double>::betaFromDecibels(parameters.getDynRange()));
	k.generate(fftSize);

	for(const std::string& inputFilename : parameters.getInputFiles()) {
		std::cout << "Opening input file: " << inputFilename << " ... ";
		Sndspec::Reader<double> r(inputFilename, fftSize, parameters.getImgWidth());

		if(r.getSndFileHandle() == nullptr || r.getSndFileHandle()->error() != SF_ERR_NO_ERROR) {
			std::cout << "couldn't open file !" << std::endl;
		} else {
			std::cout << "ok" << std::endl;
			r.setWindow(k.getData());

			// create output storage (channels x spectrums x numbins)
			std::vector<std::vector<std::vector<double>>> spectrogram(r.getNChannels(), std::vector<std::vector<double>>(parameters.getImgWidth(), std::vector<double>(spectrumSize, 0.0)));

			// create a spectrum analyzer for each channel
			std::vector<std::unique_ptr<Spectrum>> analyzers;
			for(int ch = 0; ch < r.getNChannels(); ch ++) {
				analyzers.emplace_back(new Spectrum(fftSize));

				// give the reader direct write-access to the analyzer input buffer
				r.setChannelBuffer(ch, analyzers.back()->getTdBuf());
			}

			// create a callback function to execute spectrum analysis for each block read
			r.setProcessingFunc([&analyzers, &spectrogram](int pos, int channel, const double* data) -> void {
				std::cout << "pos " << pos << std::endl;
				Spectrum* analyzer = analyzers.at(static_cast<decltype(analyzers)::size_type>(channel)).get();
				assert(data == analyzer->getTdBuf());
				analyzer->exec();
				analyzer->getMag(spectrogram[channel][pos]);
			});

			r.readDeinterleaved();

			// plot results
//			for(int x = 0; x < spectrogram.at(0).size(); x++) {
//				std::cout << "showing fft number " << x;
//				UglyPlot::plot(spectrogram.at(0).at(x).data(), spectrumSize);
//			}

		} // ends successful file-open
	} // ends loop over files
}
