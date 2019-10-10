#ifndef RENDERER_H
#define RENDERER_H

#include "parameters.h"
#include "spectrogram.h"

extern "C" {
#include <cairo.h>
}

namespace Sndspec {

class Renderer
{
public:
	Renderer(int width, int height);
	~Renderer();

	void Render(const Parameters& parameters, const SpectrogramResults<double>& spectrogramData);

	std::vector<int32_t> getHeatMapPalette() const;
	void setHeatMapPalette(const std::vector<int32_t> &value);
	bool writeToFile(const std::string &filename);

private:
	int width;
	int height;
	std::vector<uint32_t> pixelBuffer;
	int stride32;
	cairo_surface_t* surface;
	std::vector<int32_t> heatMapPalette {
		0x00ffffff,
		0x00f0fed8,
		0x00f2fbb9,
		0x00fdf58f,
		0x00fdc866,
		0x00fc9042,
		0x00fc4b20,
		0x00ed1c29,
		0x00d60340,
		0x00b70365,
		0x009d037a,
		0x007a037e,
		0x0050026e,
		0x002d0259,
		0x00130246,
		0x00010335,
		0x00010325,
		0x00010213,
		0x00000000
	};
};

} // namespace Sndspec

#endif // RENDERER_H
