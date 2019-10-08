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

private:
	int width;
	int height;
	int stride;
	unsigned char* data;
	cairo_surface_t* surface;
};

} // namespace Sndspec

#endif // RENDERER_H
