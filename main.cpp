#include "spectrogram.h"
#include "parameters.h"
#include "window.h"

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
}
