#include "spectrogram.h"
#include "parameters.h"
#include "window.h"
#include "reader.h"

#include <iostream>

#include <sndfile.hh>

int main(int argc, char** argv)
{
	(void)argc; // unused
	(void)argv; // unused

	// acquire parameters
	// todo: from args
	Sndspec::Parameters parameters;
	parameters.setInputFiles({"/tmp/testfile.wav", "/tmp/guitar.flac"});
	parameters.setFftSize(512);

	// make a suitable Kaiser window
	Sndspec::KaiserWindow<double> k;
	k.setBeta(Sndspec::KaiserWindow<double>::betaFromDecibels(parameters.getDynRange()));
	k.generate(parameters.getFftSize());

	for(const std::string& inputFilename : parameters.getInputFiles()) {
		std::cout << "Opening input file: " << inputFilename << std::endl;
		Sndspec::Reader<double> r(inputFilename, [](int64_t pos, int ch, const double* data) -> void {
			(void)data; // unused
			std::cout << "pos " << pos << " ch " << ch << std::endl;
		}, parameters.getFftSize(), parameters.getImgWidth());

		if(r.getSndFileHandle() == nullptr) {
			std::cout << "problem opening file" << std::endl;
		} else {
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
