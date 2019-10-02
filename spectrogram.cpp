#include "spectrogram.h"
#include "window.h"
#include "reader.h"
#include "spectrum.h"
#include "uglyplot.h"

#include <iostream>
#include <cassert>

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
				Spectrum* analyzer = spectrumAnalyzers.at(static_cast<decltype(spectrumAnalyzers)::size_type>(channel));
				assert(data == analyzer->getTdBuf());
				analyzer->exec();
				//const auto& mag =
				analyzer->getMag();
				//UglyPlot::plot(mag.data(), mag.size());
				auto pkinfo = analyzer->getFdPeak();
				std::cout << "peak " << pkinfo.first << " bin " << pkinfo.second << std::endl;
			});

			r.readDeinterleaved();

			// clean up
			for(auto s : spectrumAnalyzers) {
				delete s;
			}
		}
	}
}
