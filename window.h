#ifndef WINDOW_H
#define WINDOW_H

#include "factorial.h"

#include <vector>
#include <cmath>

namespace Sndspec {

// base class

template <typename FloatType>
class Window
{
public:
	Window() = default;

	const std::vector<FloatType>& getData() const
	{
		return data;
	}

	virtual void generate(int size) = 0;

	// in-place application of window to given input
	void apply(std::vector<FloatType>& input) const
	{
		for(int i = 0; i < input.size(); i++) {
			input[i] *= data.at(i);
		}
	}

protected:
	std::vector<FloatType> data;
};

// Kaiser Window class

template <typename FloatType>
class KaiserWindow : Window<FloatType>
{
	FloatType beta;

public:
	KaiserWindow() : Window<FloatType>() {}
	FloatType getBeta() const { return beta; }
	void setBeta(const FloatType &value) { beta = value; }
	void generate(int size) override {
		Window<FloatType>::data.resize(size, 0.0);
		for (int n = 0; n < size; ++n) {
			Window<FloatType>::data[n] = I0(beta * sqrt(1.0 - pow((2.0 * n / (size - 1) - 1), 2.0))) / I0(beta);
		}
	}

	static FloatType calcKaiserBeta(FloatType dB)
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
