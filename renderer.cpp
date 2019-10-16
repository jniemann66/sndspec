#include "renderer.h"

#include <inttypes.h>

#include <cstdlib>
#include <cstddef>
#include <cstdint>

namespace Sndspec {

Renderer::Renderer(int width, int height) : width(width), height(height), pixelBuffer(width * height, 0)
{
	const cairo_format_t cairoFormat =  CAIRO_FORMAT_RGB24;
	int stride =  cairo_format_stride_for_width(cairoFormat, width);
	stride32 = stride / sizeof(uint32_t);
	surface = cairo_image_surface_create_for_data(reinterpret_cast<unsigned char*>(pixelBuffer.data()), cairoFormat, width, height, stride);
	cr = cairo_create(surface);
}

Renderer::~Renderer()
{
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}

void Renderer::Render(const Parameters &parameters, const SpectrogramResults<double> &spectrogramData)
{
	int numChannels = spectrogramData.size();
	int numSpectrums = spectrogramData.at(0).size();
	int numBins = spectrogramData.at(0).at(0).size();
	int h = height - 1;
	double colorScale = heatMapPalette.size() / -parameters.getDynRange();
	int lastColorIndex = std::max(0, static_cast<int>(heatMapPalette.size()) - 1);

	for(int c = 0; c < 1 /*numChannels*/; c++) {
		for(int x = 0; x < numSpectrums; x++) {
			for(int y = 0; y < numBins; y++) {
				int addr = x + (h - y) * stride32;
				int colorindex = spectrogramData[c][x][y] * colorScale;
				int32_t color = heatMapPalette[std::max(0, std::min(colorindex, lastColorIndex))];
				pixelBuffer[addr] = color;
			}
		}
	}
}

void Renderer::drawGrid(double nyquist, double major, double minor)
{
	cairo_set_line_width (cr, 1.5);
	cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);

	int yMajStep = height * major / nyquist;
	for(int y = height - 1; y >= 0; y -= yMajStep ) {
		cairo_move_to(cr, 0, y);
		cairo_line_to(cr, width - 1, y);
	}

	int xStep = width / 5.0;
	for(int x = 0; x < width; x += xStep) {
		cairo_move_to(cr, x, 0);
		cairo_line_to(cr, x, height - 1);
	}

//	if(minor > 1.0) {
//	//	static const double dashed1[] = {4.0, 21.0, 2.0};
//	//	cairo_set_dash(cr, dashed1, 4, 0);
//		int yMinStep = height * minor / nyquist;
//		for(int y = height - 1; y >= 0; y -= yMinStep ) {
//			cairo_move_to(cr, 0, y);
//			cairo_line_to(cr, width - 1, y);
//		}
//	}

	cairo_stroke (cr);
}

bool Renderer::writeToFile(const std::string& filename)
{
	return (cairo_surface_write_to_png(surface, filename.c_str()) == CAIRO_STATUS_SUCCESS);
}

std::vector<int32_t> Renderer::getHeatMapPalette() const
{
	return heatMapPalette;
}

void Renderer::setHeatMapPalette(const std::vector<int32_t> &value)
{
	heatMapPalette = value;
}

} // namespace Sndspec
