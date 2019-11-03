#ifndef WINDOW_H
#define WINDOW_H

#include "factorial.h"

#include <vector>
#include <cmath>

namespace Sndspec {

template <typename FloatType>
class Window
{
public:
	Window() = default;

	void generateRectangular(int size)
	{
		data.resize(size, 1.0);
	}

	void generateKaiser(int size, FloatType beta) {
		data.resize(size, 0.0);
		for (int n = 0; n < size; ++n) {
			data[n] = I0(beta * sqrt(1.0 - pow((2.0 * n / (size - 1) - 1), 2.0))) / I0(beta);
		}
	}

	void generateHann(int size)
	{
		return generate1TermCosine(size, 0.5, 0.5);
	}

	void generateHamming(int size)
	{
		return generate1TermCosine(size, 0.54, 0.46);
	}

	void generateBlackman(int size)
	{
		return generate2TermCosine(size, 0.42, 0.5, 0.08);
	}

	void generateNuttall(int size)
	{
		return generate3TermCosine(size, 0.355768, 0.487396, 0.144232, 0.012604);
	}

	void generateBlackmanNuttall(int size)
	{
		return generate3TermCosine(size, 0.3635819, 0.4891775, 0.1365995, 0.0106411);
	}

	void generateBlackmanHarris(int size)
	{
		return generate3TermCosine(size, 0.35875, 0.48829, 0.14128, 0.01168);
	}

	void generateFlatTop(int size)
	{
		return generate4TermCosine(size, 0.21557895, 0.41663158, 0.277263158, 0.083578947, 0.006947368);
	}

	// generalized cosine windows
	void generate1TermCosine(int size, FloatType a0, FloatType a1)
	{
		Window<FloatType>::data.resize(size, 0.0);
		for (int n = 0; n < size; ++n) {
			data[n] = a0 - a1 * std::cos((2.0 * M_PI * n) / (size - 1));
		}
	}

	void generate2TermCosine(int size, FloatType a0, FloatType a1, FloatType a2)
	{
		Window<FloatType>::data.resize(size, 0.0);
		for (int n = 0; n < size; ++n) {
			data[n] = a0
					- a1 * std::cos((2.0 * M_PI * n) / (size - 1))
					+ a2 * std::cos((4.0 * M_PI * n) / (size - 1))
			;
		}
	}

	void generate3TermCosine(int size, FloatType a0, FloatType a1, FloatType a2, FloatType a3)
	{
		Window<FloatType>::data.resize(size, 0.0);
		for (int n = 0; n < size; ++n) {
			data[n] = a0
					- a1 * std::cos((2.0 * M_PI * n) / (size - 1))
					+ a2 * std::cos((4.0 * M_PI * n) / (size - 1))
					- a3 * std::cos((6.0 * M_PI * n) / (size - 1))
			;
		}
	}

	void generate4TermCosine(int size, FloatType a0, FloatType a1, FloatType a2, FloatType a3, FloatType a4)
	{
		Window<FloatType>::data.resize(size, 0.0);
		for (int n = 0; n < size; ++n) {
			data[n] = a0
					- a1 * std::cos((2.0 * M_PI * n) / (size - 1))
					+ a2 * std::cos((4.0 * M_PI * n) / (size - 1))
					- a3 * std::cos((6.0 * M_PI * n) / (size - 1))
					+ a4 * std::cos((8.0 * M_PI * n) / (size - 1))
			;
		}
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
