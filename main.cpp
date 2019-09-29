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

	Sndspec::Parameters parameters;
	parameters.setInputFiles({"/tmp/testfile.wav"});

	Sndspec::KaiserWindow<double> k;
	k.setBeta(Sndspec::KaiserWindow<double>::betaFromDecibels(parameters.getDynRange()));
	k.generate(parameters.getFftSize());

	Sndspec::Reader<double> r(parameters.getInputFiles().at(0), [](int64_t pos, int ch, const double* data) -> void{
		std::cout << "pos " << pos << " ch " << ch << " dataptr " << data << std::endl;
	}, 512, 4 );

	if(r.getSndFileHandle() == nullptr) {
		std::cout << "problem opening file" << std::endl;
	} else {
		r.setWindow(k.getData());
		std::vector<std::unique_ptr<double[]>> channelBuffers;
		for(int ch = 0; ch < r.getNChannels(); ch ++) {
			double* p = new double[512];
			channelBuffers.emplace_back(p);
			r.setChannelBuffer(ch, p);
		}
		r.read();
	}
}
