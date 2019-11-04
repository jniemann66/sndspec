#include "spectrogram.h"
#include "parameters.h"

#include "tests.h"

#include <iostream>

#include <sndfile.hh>

int main(int argc, char** argv)
{
//	tests::testWindow();
//	exit(0);

	Sndspec::Parameters parameters;
	std::string msg(parameters.fromArgs({argv + 1, argv + argc}));
	if(!msg.empty()) {
		std::cout << msg << std::endl;
		return 0;
	}

	Sndspec::Spectrogram::makeSpectrogram(parameters);

}
