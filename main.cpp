#include "spectrogram.h"
#include "parameters.h"
#include "uglyplot.h"

#include <iostream>

#include <sndfile.hh>

int main(int argc, char** argv)
{
	(void)argc; // unused
	(void)argv; // unused

//	double t[80];
//	for(int x = 0; x < 80; x++) {
//		t[x] = 0.001 * (x - 40);
//	}
//	UglyPlot::plot(t, 80);
//	exit(0);

	// acquire parameters
	// todo: from args
	Sndspec::Parameters parameters;
	parameters.setInputFiles({"/tmp/testfile.wav", "/tmp/guitar.flac"});
	parameters.setFftSize(2048);

	Sndspec::Spectrogram::makeSpectrogram(parameters);
}
