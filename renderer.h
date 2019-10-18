#ifndef RENDERER_H
#define RENDERER_H

#include "parameters.h"
#include "spectrogram.h"

#include <cairo.h>

extern "C" {
#include <cairo.h>
}

namespace Sndspec {

class Renderer
{
public:
	Renderer(int width, int height, double marginLeft = 0.08, double marginTop = 0.06, double marginRight = 0.08, double marginBottom = 0.06);
	~Renderer();

	void Render(const Parameters& parameters, const SpectrogramResults<double>& spectrogramData);

	std::vector<int32_t> getHeatMapPalette() const;
	void setHeatMapPalette(const std::vector<int32_t> &value);
	bool writeToFile(const std::string &filename);
	void drawGrid(double nyquist, double div);
	void drawBorder();

	int getPlotWidth() const;
	int getPlotHeight() const;

	void drawTickmarks(double nyquist, double div);
private:
	int width;
	int height;
	int plotWidth;
	int plotHeight;
	int plotOriginX;
	int plotOriginY;
	double marginLeft;
	double marginTop;
	double marginRight;
	double marginBottom;
	std::vector<uint32_t> pixelBuffer;
	int stride32;
	cairo_surface_t* surface;
	cairo_t* cr;
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
