#include "renderer.h"
#include "raiitimer.h"

#include <inttypes.h>

#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cmath>

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

void Renderer::render(const Parameters &parameters, const SpectrogramResults<double> &spectrogramData)
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

void Renderer::drawGrid(double nyquist, double div, double beginTime, double endTime, int n)
{
	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);

	double yStep = plotHeight * div / nyquist;
	double y = plotOriginY + plotHeight - 1 ;

	while(y > plotOriginY) {
		cairo_move_to(cr, plotOriginX, y);
		cairo_line_to(cr, plotOriginX + plotWidth - 1, y);
		y -= yStep;
	}

	double fWidth = static_cast<double>(plotWidth);
	double xStep = fWidth / n;
	double x = plotOriginX;

	while(x < fWidth) {
		cairo_move_to(cr, x, plotOriginY);
		cairo_line_to(cr, x, plotOriginY + plotHeight - 1);
		x += xStep;
	}

	cairo_stroke (cr);
}

void Renderer::drawBorder()
{
	cairo_set_line_width (cr, 2);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	int s = -1;
	cairo_move_to(cr, plotOriginX - s, plotOriginY - s);
	cairo_line_to(cr, plotOriginX + plotWidth + s, plotOriginY - s);
	cairo_line_to(cr, plotOriginX + plotWidth + s, plotOriginY + plotHeight + s);
	cairo_line_to(cr, plotOriginX - s,  plotOriginY + plotHeight + s);
	cairo_close_path(cr);

	cairo_stroke(cr);
}

void Renderer::drawTickmarks(double nyquist, double div, double beginTime, double endTime, int n)
{
	cairo_set_line_width (cr, 2);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

	int s = 10;
	int fx = s + 5;
	int fy = 4;

	cairo_set_font_size(cr, 13);

	double yStep = plotHeight * div / nyquist;
	double y = plotOriginY + plotHeight - 1 ;
	int f = 0;

	char fLabelBuf[20];

	while(y > plotOriginY) {
		sprintf(fLabelBuf, "%d", static_cast<int>(f));
//		itoa(static_cast<int>(f), fLabelBuf, 10);
		cairo_move_to(cr, plotOriginX + plotWidth, y);
		cairo_line_to(cr, plotOriginX + s + plotWidth - 1, y);
		cairo_move_to(cr, plotOriginX + fx + plotWidth - 1, y + fy);
		cairo_show_text(cr, fLabelBuf);
		y -= yStep;
		f += div;
	}

	double fWidth = static_cast<double>(plotWidth);
	double xStep = fWidth / n;
	double x = plotOriginX;

	char tLabelBuf[20];
	double tStep = (endTime - beginTime) / n;
	double t = 0.0;
	int tx = -5;
	int ty = s + 15;

	while(x < fWidth) {
		sprintf(tLabelBuf, "%6.3f", t);
		cairo_move_to(cr, x, plotOriginY + plotHeight);
		cairo_line_to(cr, x, plotOriginY + plotHeight + s -1);
		cairo_move_to(cr, x + tx, plotOriginY + plotHeight + ty -1);
		cairo_show_text(cr, tLabelBuf);
		x += xStep;
		t += tStep;
	}

	cairo_stroke (cr);
}

void Renderer::drawHeatMap()
{
	double sc = static_cast<double>(heatMapPalette.size()) / plotHeight;

	int x0 = 10;
	int w = 10;

	for(int y = 0; y < plotHeight; y++) {
		int lineAddr = x0 + (plotOriginY + y) * stride32;
		int32_t color = heatMapPalette.at(static_cast<int>(sc * y));
		for (int x = 0; x < w; x++) {
			pixelBuffer[x + lineAddr] = color;
		}
	}

	double s = 0.5;
	cairo_set_source_rgb(cr, 255, 255, 255);
	cairo_set_line_width (cr, 2);
	cairo_rectangle(cr, x0-s, plotOriginY - s, w + 2 * s, plotHeight + 2 * s);
	cairo_stroke(cr);

}


void Renderer::drawText(const std::string& heading, const std::string& info, const std::string& horizAxis, const std::string& vertAxis)
{
	double s = 20.0;

	// heading
	cairo_text_extents_t headingTextExtents;
	cairo_set_font_size(cr, 16);
	cairo_text_extents(cr, heading.c_str(), &headingTextExtents);
	cairo_move_to(cr, plotOriginX + (plotWidth - headingTextExtents.x_advance) / 2.0, s); // place at center of plot area
	cairo_show_text(cr, heading.c_str());
	cairo_set_font_size(cr, 13);

	// info
	cairo_text_extents_t infoExtents;
	cairo_text_extents(cr, info.c_str(), &infoExtents);
	cairo_move_to(cr, plotOriginX, plotOriginY - infoExtents.height);
	cairo_show_text(cr, info.c_str());

	// horizAxis
	cairo_text_extents_t horizAxisLabelExtents;
	cairo_text_extents(cr, horizAxis.c_str(), &horizAxisLabelExtents);
	cairo_move_to(cr, plotOriginX + (plotWidth - horizAxisLabelExtents.x_advance) / 2.0, height - horizAxisLabelExtents.height);
	cairo_show_text(cr, horizAxis.c_str());

	// vertAxis
	cairo_text_extents_t vertAxisLabelExtents;
	cairo_text_extents(cr, vertAxis.c_str(), &vertAxisLabelExtents);
	cairo_save(cr);
	cairo_move_to(cr, width - s /*vertAxisLabelExtents.height */, plotOriginY + (plotHeight) / 2.0);
	cairo_rotate(cr, M_PI_2);

	cairo_show_text(cr, vertAxis.c_str());
	cairo_restore(cr);

}


void Renderer::clear()
{
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_paint(cr);
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
