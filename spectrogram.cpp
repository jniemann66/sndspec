#include "spectrogram.h"
#include "window.h"
#include "reader.h"

#include <iostream>

void Sndspec::Spectrogram::makeSpectrogram(const Sndspec::Parameters &parameters)
{
	// make a suitable Kaiser window
	Sndspec::KaiserWindow<double> k;
	k.setBeta(Sndspec::KaiserWindow<double>::betaFromDecibels(parameters.getDynRange()));
	k.generate(parameters.getFftSize());

	for(const std::string& inputFilename : parameters.getInputFiles()) {
		std::cout << "Opening input file: " << inputFilename << " ... ";
		Sndspec::Reader<double> r(inputFilename, [](int64_t pos, int ch, const double* data) -> void {
			(void)data; // unused
			std::cout << "pos " << pos << " ch " << ch << std::endl;
		}, parameters.getFftSize(), parameters.getImgWidth());

		if(r.getSndFileHandle() == nullptr || r.getSndFileHandle()->error() != SF_ERR_NO_ERROR) {
			std::cout << "couldn't open file !" << std::endl;
		} else {
			std::cout << "ok" << std::endl;
			r.setWindow(k.getData());
			std::vector<std::unique_ptr<double[]>> channelBuffers;
			for(int ch = 0; ch < r.getNChannels(); ch ++) {
				double* p = new double[static_cast<size_t>(parameters.getFftSize())];
				channelBuffers.emplace_back(p);
				r.setChannelBuffer(ch, p);
			}
			r.read();
		}
	}
}
