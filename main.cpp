#include "spectrogram.h"
#include "parameters.h"
#include "window.h"

#include <iostream>

#include <sndfile.hh>

int main(int argc, char** argv)
{
    std::cout << "fuck you, world" << std::endl;

//	Sndspec::Parameters parameters;
//	parameters.setInputFiles({"/tmp/testfile.wav"});

	Sndspec::KaiserWindow<double> k;
	k.setBeta(Sndspec::KaiserWindow<double>::calcKaiserBeta(95));
	k.generate(512);
}
