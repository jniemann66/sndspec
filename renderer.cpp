#include "renderer.h"

#include <inttypes.h>

#include <cstdlib>
#include <cstddef>
#include <cstdint>

namespace Sndspec {

Renderer::Renderer(int width, int height, double marginLeft, double marginTop, double marginRight, double marginBottom)
	: width(width), height(height), marginLeft(marginLeft), marginTop(marginTop), marginRight(marginRight), marginBottom(marginBottom), pixelBuffer(width * height, 0)
{

	plotWidth = width - static_cast<int>(marginLeft * width) - static_cast<int>(marginRight * width);
	plotHeight = height - static_cast<int>(marginTop * height) - static_cast<int>(marginBottom * height);
	plotOriginX = static_cast<int>(marginLeft * width);
	plotOriginY = static_cast<int>(marginTop * height);

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
	int h = plotHeight - 1;
	double colorScale = heatMapPalette.size() / -parameters.getDynRange();
	int lastColorIndex = std::max(0, static_cast<int>(heatMapPalette.size()) - 1);

	for(int c = 0; c < 1 /*numChannels*/; c++) {
		for(int y = 0; y < numBins; y++) {
			int lineAddr = plotOriginX + (plotOriginY + h - y) * stride32;
			for(int x = 0; x < numSpectrums; x++) {
				int colorindex = spectrogramData[c][x][y] * colorScale;
				int32_t color = heatMapPalette[std::max(0, std::min(colorindex, lastColorIndex))];
				pixelBuffer[x + lineAddr] = color;
			}
		}
	}
}

void Renderer::drawGrid(double nyquist, double div)
{
	cairo_set_line_width (cr, 1.5);
	cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);

	double yStep = plotHeight * div / nyquist;
	double y = plotOriginY + plotHeight - 1 ;

	while(y > plotOriginY) {
		cairo_move_to(cr, plotOriginX, y);
		cairo_line_to(cr, plotOriginX + plotWidth - 1, y);
		y -= yStep;
	}

	double fWidth = static_cast<double>(plotWidth);
	double xStep = fWidth / 5.0;
	double x = plotOriginX;

	while(x < fWidth) {
		cairo_move_to(cr, x, plotOriginY);
		cairo_line_to(cr, x, plotOriginY + plotHeight - 1);
		x += xStep;
	}

	cairo_stroke (cr);
}

int Renderer::getPlotWidth() const
{
	return plotWidth;
}

int Renderer::getPlotHeight() const
{
	return plotHeight;
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
