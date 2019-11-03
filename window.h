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
		return generalizedCosineWindow(size, {0.5, 0.5} );
	}

	void generateHamming(int size)
	{
		return generalizedCosineWindow(size, {0.54, 0.46} );
	}

	void generateBlackman(int size)
	{
		return generalizedCosineWindow(size, {0.42, 0.5, 0.08} );
	}

	void generateNuttall(int size)
	{
		return generalizedCosineWindow(size, {0.355768, 0.487396, 0.144232, 0.012604} );
	}

	void generateBlackmanNuttall(int size)
	{
		return generalizedCosineWindow(size, {0.3635819, 0.4891775, 0.1365995, 0.0106411} );
	}

	void generateBlackmanHarris(int size)
	{
		return generalizedCosineWindow(size, {0.35875, 0.48829, 0.14128, 0.01168} );
	}

	void generateFlatTop(int size)
	{
		return generalizedCosineWindow(size, {0.21557895, 0.41663158, 0.277263158, 0.083578947, 0.006947368} );
	}

	void generateWindow1(int size)
	{
		return generalizedCosineWindow(size, {
									   2.374298741532465928226E-01,
									   3.994704373801009358001E-01,
									   2.362644608100282475133E-01,
									   9.620676838363516649024E-02,
									   2.591512168016078991738E-02,
									   4.307708101213669512442E-03,
									   3.904113541372495568636E-04,
									   1.508613505022821880403E-05,
									   1.320024271202038321705E-07
								   });
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
				s = -s;
			}
			data[n] = a;
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
