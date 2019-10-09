#include "renderer.h"

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

std::vector<int32_t> Renderer::getHeatMapPalette() const
{
	return heatMapPalette;
}

void Renderer::setHeatMapPalette(const std::vector<int32_t> &value)
{
	heatMapPalette = value;
}


} // namespace Sndspec
