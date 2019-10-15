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
}

Renderer::~Renderer()
{
	cairo_surface_destroy(surface);
}

void Renderer::Render(const Parameters &parameters, const SpectrogramResults<double> &spectrogramData)
{
	int numChannels = spectrogramData.size();
	int numSpectrums = spectrogramData.at(0).size();
	int numBins = spectrogramData.at(0).at(0).size();
	int h = height - 1;
	double colorScale = heatMapPalette.size() / -parameters.getDynRange();
	size_t lastColorIndex = std::max(static_cast<size_t>(0), static_cast<size_t>(heatMapPalette.size()) - 1);

	for(int c = 0; c < 1 /*numChannels*/; c++) {
		for(int x = 0; x < numSpectrums; x++) {
			for(int y = 0; y < numBins; y++) {
				int addr = x + (h - y) * stride32;
				size_t colorindex = spectrogramData[c][x][y] * colorScale;
				int32_t color = heatMapPalette[std::max(static_cast<size_t>(0), std::min(colorindex, lastColorIndex))];
				pixelBuffer[addr] = color;
			}
		}
	}
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
