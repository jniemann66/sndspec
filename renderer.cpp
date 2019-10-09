#include "renderer.h"

#include <inttypes.h>

#include <cstdlib>

namespace Sndspec {

Renderer::Renderer(int width, int height) : width(width), height(height)
{
	const cairo_format_t cairoFormat =  CAIRO_FORMAT_RGB24;
	stride = cairo_format_stride_for_width(cairoFormat, width);
	data = (unsigned char*)std::malloc(stride * height);
	surface = cairo_image_surface_create_for_data (data, cairoFormat, width, height, stride);
}

Renderer::~Renderer()
{
	cairo_surface_destroy(surface);
	free(data);
}

void Renderer::Render(const Parameters &parameters, const SpectrogramResults<double> &spectrogramData)
{
	int numChannels = spectrogramData.size();
	int numSpectrums = spectrogramData.at(0).size();
	int numBins = spectrogramData.at(0).at(0).size();

	double colorScale = heatMapPalette.size() / -parameters.getDynRange();

	for(int c = 0; c < 1 /*numChannels*/; c++) {
		for(int x = 0; x < numSpectrums; x++) {
			for(int y = 0; y < numBins; y++) {
				int addr = y * stride + x * 4;
				int32_t color = heatMapPalette[spectrogramData[c][x][y] * colorScale];
				data[addr] = color;
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
