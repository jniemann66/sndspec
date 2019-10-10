#include "spectrogram.h"
#include "parameters.h"
#include "uglyplot.h"

#include <iostream>

#include <sndfile.hh>

int main(int argc, char** argv)
{
	(void)argc; // unused
	(void)argv; // unused

	// acquire parameters
	// todo: from args
	Sndspec::Parameters parameters;

#ifdef WIN32
	parameters.setInputFiles({"e:\\TestWav\\96khz_sweep-3dBFS_32f.wav"});
#else
	parameters.setInputFiles({"/tmp/testfile.wav", "/tmp/guitar.flac"});
#endif

	parameters.setFftSize(1024);
	Sndspec::Spectrogram::makeSpectrogram(parameters);
}
