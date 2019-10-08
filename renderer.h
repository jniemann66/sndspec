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
    static void Render(const Parameters& parameters, const SpectrogramResults<double>& data);
};

} // namespace Sndspec

#endif // RENDERER_H
