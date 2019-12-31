/*
* Copyright (C) 2019 - 2020 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef SPECTROGRAM_H
#define SPECTROGRAM_H

#include "parameters.h"

namespace Sndspec {

template <typename T>
using SpectrogramResults = std::vector<std::vector<std::vector<T>>>; // (channels x spectrums x numbins)

class Spectrogram {

public:
	static void makeSpectrogramFromFile(const Parameters& parameters);
	static std::vector<bool> convertToDb(SpectrogramResults<double>& s, bool fromMagSquared = true); // return value indicates whether each channel has a signal (ie not silent)

private:

};

} // namespace Sndspec

#endif
