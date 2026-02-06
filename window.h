/*
* Copyright (C) 2019 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef WINDOW_H
#define WINDOW_H

#include "factorial.h"

#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <string>

#include <iostream>

namespace Sndspec {

enum WindowType
{
	Unknown,
	Rectangular,
	Bartlett,
	Triangular,
    Welch,
	CosineSum,
	Kaiser
};

struct WindowParameters
{
	std::string name;
	std::string displayName;
	WindowType windowType;
	std::vector<double> coefficients;
};

static const std::vector<WindowParameters> windowDefinitions
{
	{"rectangular",		"Rectangular", Rectangular, {}},
	{"bartlett",		"Bartlett", Bartlett, {}},
	{"triangular",		"Triangular", Triangular, {}},
    {"welch",           "Welch", Welch, {}},
	{"hann",			"Hann", CosineSum, {0.5, 0.5}},
	{"hanning",			"Hanning", CosineSum, {0.5, 0.5}},
	{"hamming",			"Hamming", CosineSum, {0.54, 0.46}},
	{"blackman",		"Blackman", CosineSum, {0.42, 0.5, 0.08}},
	{"nuttall",			"Nuttall", CosineSum, {0.355768, 0.487396, 0.144232, 0.012604}},
	{"blackmannuttall",	"Blackman-Nuttall", CosineSum, {0.3635819, 0.4891775, 0.1365995, 0.0106411}},
	{"blackmanharris",	"Blackman-Harris", CosineSum, {0.35875, 0.48829, 0.14128, 0.01168}},
	{"flattop",			"Flat top", CosineSum, {0.21557895, 0.41663158, 0.277263158, 0.083578947, 0.006947368}},
	{"5term", "5-term", CosineSum, {
			3.232153788877343e-01,
			4.714921439576260e-01,
			1.755341299601972e-01,
			2.849699010614994e-02,
			1.261357088292677e-03
		}}, // (1)
	{"6term", "6-term", CosineSum, {
			2.935578950102797e-01,
			4.519357723474506e-01,
			2.014164714263962e-01,
			4.792610922105837e-02,
			5.026196426859393e-03,
			1.375555679558877e-04
		}},
	{"7term", "7-term", CosineSum, {
			2.712203605850388e-001,
			4.334446123274422e-001,
			2.180041228929303e-001,
			6.578534329560609e-002,
			1.076186730534183e-002,
			7.700127105808265e-004,
			1.368088305992921e-005
		}},
	{"8term", "8-term", CosineSum, {
			2.533176817029088e-001,
			4.163269305810218e-001,
			2.288396213719708e-001,
			8.157508425925879e-002,
			1.773592450349622e-002,
			2.096702749032688e-003,
			1.067741302205525e-004,
			1.280702090361482e-006
		}},
	{"9term", "9-term", CosineSum, {
			2.384331152777942e-001,
			4.005545348643820e-001,
			2.358242530472107e-001,
			9.527918858383112e-002,
			2.537395516617152e-002,
			4.152432907505835e-003,
			3.685604163298180e-004,
			1.384355593917030e-005,
			1.161808358932861e-007,
		}},
	{"10term", "10-term", CosineSum, {
			2.257345387130214e-001,
			3.860122949150963e-001,
			2.401294214106057e-001,
			1.070542338664613e-001,
			3.325916184016952e-002,
			6.873374952321475e-003,
			8.751673238035159e-004,
			6.008598932721187e-005,
			1.710716472110202e-006,
			1.027272130265191e-008,
		}},
	{"11term", "11-term", CosineSum, {
			2.151527506679809e-001,
			3.731348357785249e-001,
			2.424243358446660e-001,
			1.166907592689211e-001,
			4.077422105878731e-002,
			1.000904500852923e-002,
			1.639806917362033e-003,
			1.651660820997142e-004,
			8.884663168541479e-006,
			1.938617116029048e-007,
			8.482485599330470e-010
		}},
	{"kaiser", "Kaiser", Kaiser, {}}
};

static std::vector<std::string> getWindowNames()
{
	std::vector<std::string> names;
	for (const auto& w : windowDefinitions) {
		names.push_back(w.displayName);
	}
	return names;
}

template <typename FloatType>
class Window
{

public:
	Window() = default;

	void generateRectangular(int size)
	{
		data.resize(size, 1.0);
	}

    // Triangular: first and last elements are NOT zero
	void generateTriangular(int size)
	{
		data.resize(size, 0.0);
        for (int n = 0 ; n < size; n++) {
			data[n] = 1.0 - std::abs((n - (size - 1) / 2.0) / ((size + 1) / 2.0));
		}
	}

    // Bartlett: first and last elements are zero
	void generateBartlett(int size)
	{
		data.resize(size, 0.0);
        for (int n = 0 ; n < size; n++) {
			data[n] = 1.0 - std::abs((n - (size - 1) / 2.0) / ((size - 1) / 2.0));
		}
	}

    void generateWelch(int size)
    {
        data.resize(size, 0.0);
        const double half_sizeMinus1 = static_cast<double>(size - 1) / 2.0;
        for (int n = 0; n < size; n++) {
            double t = (static_cast<double>(n) - half_sizeMinus1) / half_sizeMinus1;
            data[n] = 1 - t * t;
        }
    }

	void generalizedCosineWindow(int size, std::vector<FloatType> coeffs)
	{
		int N = size;
		int K = coeffs.size();

		Window<FloatType>::data.resize(size, 0.0);
		for (int n = 0; n < size; ++n) {
			FloatType s = 1.0; // sign
			FloatType a = 0.0; // accumulator
			for (int k = 0; k < K; k++) {
				a += s * coeffs.at(k) * std::cos(2 * M_PI * k * n / (N - 1));
				s = -s; // flip sign
			}
			data[n] = a;
		}
	}

	void generateKaiser(int size, FloatType beta) {
		data.resize(size, 0.0);
		for (int n = 0; n < size; ++n) {
			data[n] = I0(beta * sqrt(1.0 - pow((2.0 * n / (size - 1) - 1), 2.0))) / I0(beta);
		}
	}

	void generate(std::string name, int size, FloatType val = 0.0)
	{
		const auto w = findWindow(name);
		switch (w.windowType)
		{
		case Rectangular:
			return generateRectangular(size);
		case Bartlett:
			return generateBartlett(size);
		case Triangular:
            return generateTriangular(size);
        case Welch:
            return generateWelch(size);
		case CosineSum:
			return generalizedCosineWindow(size, w.coefficients);
		case Kaiser:
		case Unknown:
			return generateKaiser(size, val);
		}
	}

	static WindowParameters findWindow(std::string name)
	{
		// remove non-alphanum characters from name
		name.erase(std::remove_if(name.begin(), name.end(), [](unsigned char c) -> bool {
			return !std::isalnum(c);
		}), name.end());

		// convert name to lowercase
		std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) -> unsigned char {
			return std::tolower(c);
		});

		for (const auto& w : windowDefinitions) {
			if (w.name.compare(name) == 0) {
				return w;
			}
		}

		return {"unknown", "Unknown", Unknown, {}};
	}


	const std::vector<FloatType>& getData() const
	{
		return data;
	}

	// in-place application of window to given input
	void apply(std::vector<FloatType>& input) const
	{
		for (int i = 0; i < input.size(); i++) {
			input[i] *= data.at(i);
		}
	}

	static FloatType kaiserBetaFromDecibels(FloatType dB)
	{
		FloatType kaiserBeta{0.0};
		if (dB < 21.0) {
			kaiserBeta = 0.0;
		} else if ((dB >= 21.0) && (dB <= 50.0)) {
			kaiserBeta = 0.5842 * pow((dB - 21), 0.4) + 0.07886 * (dB - 21);
		} else {
			kaiserBeta = 0.1102 * (dB - 8.7);
		}
		return kaiserBeta;
	}


private:
	std::vector<FloatType> data;

	// I0() : 0th-order Modified Bessel function of the first kind:
	static FloatType I0(FloatType z)
	{
		FloatType result = 0.0;
		FloatType oldResult = 0.0;
		int k = 0;
		do {
			FloatType kfact = factorial[k];
			oldResult = result;

			//	FloatType x = pow(z * z / 4.0, k) / (kfact * kfact); // square first
			//	result += x;

			FloatType x = pow(z / 2.0, k) / kfact;
			result += x * x; // square last

			k++;

		} while (std::fpclassify(result - oldResult) == FP_NORMAL);

		return result;
	}
};

} // namespace Sndspec

/* (1)
H. H. Albrecht, "A family of cosine-sum windows for high-resolution measurements,"
2001 IEEE International Conference on Acoustics, Speech, and Signal Processing. Proceedings (Cat. No.01CH37221),
Salt Lake City, UT, USA, 2001, pp. 3081-3084 vol.5.
doi: 10.1109/ICASSP.2001.940309
*/

#endif // WINDOW_H
