#ifndef WINDOW_H
#define WINDOW_H

#include "factorial.h"

#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <cctype>

namespace Sndspec {

enum WindowType
{
	Rectangular,
	Bartlett,
	Triangular,
	CosineSum,
	Kaiser
};

struct WindowParameters
{
	WindowType windowType;
	std::vector<double> coefficients;
};

static const std::map<std::string, WindowParameters> windowDefinitions
{
	{"rectangular", {Rectangular, {}}},
	{"bartlett", {Bartlett, {}}},
	{"triangular", {Triangular, {}}},
	{"hann", {CosineSum, {0.5, 0.5}}},
	{"hanning", {CosineSum, {0.5, 0.5}}},
	{"hamming", {CosineSum, {0.54, 0.46}}},
	{"blackman", {CosineSum, {0.42, 0.5, 0.08}}},
	{"nuttall", {CosineSum, {0.355768, 0.487396, 0.144232, 0.012604}}},
	{"blackmannuttall", {CosineSum, {0.3635819, 0.4891775, 0.1365995, 0.0106411}}},
	{"blackmanharris", {CosineSum, {0.35875, 0.48829, 0.14128, 0.01168}}},
	{"5term", {CosineSum, {}}},
	{"6term", {CosineSum, {}}},
	{"7term", {CosineSum, {}}},
	{"8term", {CosineSum, {}}},
	{"9term", {CosineSum, {}}},
	{"10term", {CosineSum, {}}},
	{"11term", {CosineSum, {}}},
	{"kaiser", {Kaiser, {}}}
};

static std::vector<std::string> getWindowNames()
{
	std::vector<std::string> names;
	for(const auto& w : windowDefinitions) {
		names.push_back(w.first);
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

	void generateTriangular(int size)
	{
		data.resize(size, 0.0);
		for(int n = 0 ; n < size; n++)
		{
			data[n] = 1.0 - std::abs((n - (size - 1) / 2.0) / ((size + 1) / 2.0));
		}
	}

	void generateBartlett(int size)
	{
		data.resize(size, 0.0);
		for(int n = 0 ; n < size; n++)
		{
			data[n] = 1.0 - std::abs((n - (size - 1) / 2.0) / ((size - 1) / 2.0));
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
			for(int k = 0; k < K; k++) {
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
		// remove non-alphanum characters from name
		name.erase(std::remove_if(name.begin(), name.end(), [](unsigned char c) -> bool {
			return !std::isalnum(c);
		}), name.end());

		// convert name to lowercase
		std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) -> unsigned char {
			return std::tolower(c);
		});

		if(windowDefinitions.count(name) != 0) {
			const WindowParameters& windowDefinition = windowDefinitions.at(name);
			switch(windowDefinition.windowType) {
			case Rectangular:
				return generateRectangular(size);
			case Bartlett:
				return generateBartlett(size);
			case Triangular:
				return generateTriangular(size);
			case CosineSum:
				return generalizedCosineWindow(size, windowDefinition.coefficients);
			case Kaiser:
				return generateKaiser(size, val);
			}
		}

		return generateKaiser(size, val);
	}

	const std::vector<FloatType>& getData() const
	{
		return data;
	}

	// in-place application of window to given input
	void apply(std::vector<FloatType>& input) const
	{
		for(int i = 0; i < input.size(); i++) {
			input[i] *= data.at(i);
		}
	}

	static FloatType kaiserBetaFromDecibels(FloatType dB)
	{
		if(dB < 21.0) {
			return 0;
		}
		if ((dB >= 21.0) && (dB <= 50.0)) {
			return 0.5842 * pow((dB - 21), 0.4) + 0.07886 * (dB - 21);
		}
		if (dB > 50.0) {
			return 0.1102 * (dB - 8.7);
		}
	}



private:
	std::vector<FloatType> data;

	// I0() : 0th-order Modified Bessel function of the first kind:
	// todo: C++17 now includes this function in math library - worth checking it out

	static FloatType I0(FloatType z)
	{
		FloatType result = 0.0;
		for (int k = 0; k < 34; ++k) {
			FloatType kfact = factorial[k];
			FloatType x = pow(z * z / 4.0, k) / (kfact * kfact);
			result += x;
		}
		return result;
	}
};

} // namespace Sndspec

#endif // WINDOW_H
