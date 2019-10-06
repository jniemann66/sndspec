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

			// create output storage (channels x spectrums x fftbins)
			std::vector<std::vector<std::vector<double>>> spectrogram(r.getNChannels(), std::vector<std::vector<double>>(parameters.getImgWidth(), std::vector<double>(fftSize, 0.0)));

			// create a spectrum analyzer for each channel
			std::vector<Spectrum*> spectrumAnalyzers;

			for(int ch = 0; ch < r.getNChannels(); ch ++) {
				Spectrum* s = new Spectrum(fftSize);
				spectrumAnalyzers.push_back(s);
				r.setChannelBuffer(ch, s->getTdBuf());
			}

			// create a callback function to execute spectrum analysis for each block read
			r.setProcessingFunc([spectrumAnalyzers, &spectrogram](int pos, int channel, const double* data) -> void {
				std::cout << "pos " << pos << std::endl;
				Spectrum* analyzer = spectrumAnalyzers.at(static_cast<decltype(spectrumAnalyzers)::size_type>(channel));
				assert(data == analyzer->getTdBuf());
				analyzer->exec();
				analyzer->getMag(spectrogram[channel][pos]);
				//UglyPlot::plot(mag.data(), mag.size());
			});

			r.readDeinterleaved();

			// clean up
			for(auto s : spectrumAnalyzers) {
				delete s;
			}
		}
	}
}
