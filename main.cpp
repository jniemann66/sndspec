#include "spectrogram.h"
#include "parameters.h"

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

	Sndspec::Spectrogram::makeSpectrogram(parameters);
}
