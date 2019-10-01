#include "spectrogram.h"
#include "window.h"
#include "reader.h"
#include "spectrum.h"

#include <iostream>

void Sndspec::Spectrogram::makeSpectrogram(const Sndspec::Parameters &parameters)
{
	// make a suitable Kaiser window
	Sndspec::KaiserWindow<double> k;
	k.setBeta(Sndspec::KaiserWindow<double>::betaFromDecibels(parameters.getDynRange()));
	k.generate(parameters.getFftSize());

	for(const std::string& inputFilename : parameters.getInputFiles()) {
		std::cout << "Opening input file: " << inputFilename << " ... ";
		Sndspec::Reader<double> r(inputFilename, parameters.getFftSize(), parameters.getImgWidth());

		if(r.getSndFileHandle() == nullptr || r.getSndFileHandle()->error() != SF_ERR_NO_ERROR) {
			std::cout << "couldn't open file !" << std::endl;
		} else {
			std::cout << "ok" << std::endl;
			r.setWindow(k.getData());

			// create a spectrum analyzer for each channel
			std::vector<Spectrum*> spectrumAnalyzers;

			for(int ch = 0; ch < r.getNChannels(); ch ++) {
				Spectrum* s = new Spectrum(parameters.getFftSize());
				spectrumAnalyzers.push_back(s);
				r.setChannelBuffer(ch, s->getTdBuf());
			}

			// create a callback function to execute spectrum analysis for each block read
			r.setProcessingFunc([spectrumAnalyzers](int64_t frame, int channel, const double* data) -> void {
				std::cout << "frame " << frame << std::endl;
				//(void)frame; // unused
				(void)data; // unused
				spectrumAnalyzers.at(static_cast<decltype(spectrumAnalyzers)::size_type>(channel))->exec();
				spectrumAnalyzers.at(static_cast<decltype(spectrumAnalyzers)::size_type>(channel))->getMag();
//				if(frame == 160800) {
//					std::cout << "Booyah " << std::endl;
//				}
			});

			r.read();

			// clean up
			for(auto s : spectrumAnalyzers) {
				delete s;
			}
		}
	}
}
