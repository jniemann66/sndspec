#ifndef WINDOW_H
#define WINDOW_H

#include <vector>

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
	KaiserWindow() {
		Window<FloatType>::Window();
	}

	FloatType beta;

public:
	FloatType getBeta() const { return beta; }
	void setBeta(const FloatType &value) { beta = value; }
	void generate(int size) override;
	static FloatType calcKaiserBeta(FloatType dB);
};

template<typename FloatType>
FloatType KaiserWindow<FloatType>::calcKaiserBeta(FloatType dB)
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


} // namespace Sndspec

#endif // WINDOW_H
