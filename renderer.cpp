#include "renderer.h"
#include "raiitimer.h"

#include <inttypes.h>

#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cmath>

namespace Sndspec {

Renderer::Renderer(int width, int height) : width(width), height(height), pixelBuffer(width * height, 0)
{
	// set up cairo surface
	const cairo_format_t cairoFormat =  CAIRO_FORMAT_RGB24;
	int stride =  cairo_format_stride_for_width(cairoFormat, width);
	stride32 = stride / sizeof(uint32_t);
	surface = cairo_image_surface_create_for_data(reinterpret_cast<unsigned char*>(pixelBuffer.data()), cairoFormat, width, height, stride);
	cr = cairo_create(surface);

	// calculate dimensions of actual plot area
	setMargins();
	plotWidth = width - marginLeft - marginRight;
	plotHeight = height - static_cast<int>(marginTop) - static_cast<int>(marginBottom);
	plotOriginX = static_cast<int>(marginLeft);
	plotOriginY = static_cast<int>(marginTop);
	hmOriginY = plotOriginY; // align top of heatmap with top of plot area
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

void Renderer::drawGrid()
{
	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);

	double yStep = plotHeight * freqStep / nyquist;
	double y = plotOriginY + plotHeight - 1 ;

	while(y > plotOriginY) {
		cairo_move_to(cr, plotOriginX, y);
		cairo_line_to(cr, plotOriginX + plotWidth - 1, y);
		y -= yStep;
	}

	double fWidth = static_cast<double>(plotWidth);
	double xStep = fWidth / numTimeDivs;
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

void Renderer::drawTickmarks()
{
	cairo_set_line_width (cr, 2);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

	const int s = 10;
	constexpr int fx = s + 5;
	const int fy = 4;

	cairo_set_font_size(cr, 13);

	double yStep = plotHeight * freqStep / nyquist;
	double y = plotOriginY + plotHeight - 1 ;
	int f = 0;

	char fLabelBuf[20];

	while(y > plotOriginY) {
		sprintf(fLabelBuf, "%d", static_cast<int>(f));
		cairo_move_to(cr, plotOriginX + plotWidth, y);
		cairo_line_to(cr, plotOriginX + s + plotWidth - 1, y);
		cairo_move_to(cr, plotOriginX + fx + plotWidth - 1, y + fy);
		cairo_show_text(cr, fLabelBuf);
		y -= yStep;
		f += freqStep;
	}

	double fWidth = static_cast<double>(plotWidth);
	double xStep = fWidth / numTimeDivs;
	double x = plotOriginX;

	char tLabelBuf[20];
	double tStep = (finishTime - startTime) / numTimeDivs;
	std::cout << startTime <<" "<< finishTime <<" "<< "tStep " << tStep << std::endl;
	double t = startTime;
	const int tx = -5;
	constexpr int ty = s + 15;

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

void Renderer::drawText()
{
	double s = 20.0;

	// heading
	cairo_text_extents_t headingTextExtents;
	cairo_set_font_size(cr, 16);
	cairo_text_extents(cr, title.c_str(), &headingTextExtents);
	cairo_move_to(cr, plotOriginX + (plotWidth - headingTextExtents.x_advance) / 2.0, s); // place at center of plot area
	cairo_show_text(cr, title.c_str());
	cairo_set_font_size(cr, 13);

	// info
	cairo_text_extents_t infoExtents;
	cairo_text_extents(cr, inputFilename.c_str(), &infoExtents);
	cairo_move_to(cr, plotOriginX, plotOriginY - infoExtents.height);
	cairo_show_text(cr, inputFilename.c_str());

	// horizAxis
	cairo_text_extents_t horizAxisLabelExtents;
	cairo_text_extents(cr, horizAxisLabel.c_str(), &horizAxisLabelExtents);
	cairo_move_to(cr, plotOriginX + (plotWidth - horizAxisLabelExtents.x_advance) / 2.0, height - horizAxisLabelExtents.height);
	cairo_show_text(cr, horizAxisLabel.c_str());

	// vertAxis
	cairo_text_extents_t vertAxisLabelExtents;
	cairo_text_extents(cr, vertAxisLabel.c_str(), &vertAxisLabelExtents);
	cairo_save(cr);
	cairo_move_to(cr, width - s, plotOriginY + (plotHeight) / 2.0);
	cairo_rotate(cr, M_PI_2);
	cairo_show_text(cr, vertAxisLabel.c_str());
	cairo_restore(cr);

}

void Renderer::drawHeatMap()
{
	double sc = static_cast<double>(heatMapPalette.size()) / plotHeight;

	// draw the heatmap colours
	for(int y = 0; y < plotHeight; y++) {
		int lineAddr = hmOriginX + (plotOriginY + y) * stride32;
		int32_t color = heatMapPalette.at(static_cast<int>(sc * y));
		for (int x = 0; x < hmWidth; x++) {
			pixelBuffer[x + lineAddr] = color;
		}
	}

	// draw the heatmap border
	double s = 0.5;
	cairo_set_source_rgb(cr, 255, 255, 255);
	cairo_set_line_width (cr, 2);
	cairo_rectangle(cr, hmOriginX-s, plotOriginY - s, hmWidth + 2 * s, plotHeight + 2 * s);
	cairo_stroke(cr);

	// draw the 'dB' heading
	cairo_text_extents_t dBExtents;
	cairo_text_extents(cr, "dB", &dBExtents);
	cairo_move_to(cr, hmOriginX, plotOriginY - dBExtents.height);
	cairo_show_text(cr, "dB");

	// draw the dB tickmarks and labels
	double dB = 0.0;
	double dBsc = plotHeight / dynRange;
	double xa = hmOriginX + hmWidth + s;
	double xb = xa + tickWidth;
	char dbBuf[20];
	while (dB < dynRange) {
		sprintf(dbBuf, "%3.0f", -dB);
		double y = plotOriginY + dBsc * dB - s;
		cairo_move_to(cr, xa, y);
		cairo_line_to(cr, xb, y);
		cairo_move_to(cr, xb + 2, y + 3);
		cairo_show_text(cr, dbBuf);
		dB += 10.0;
	}

	cairo_stroke(cr);
}

void Renderer::makeNegativeImage()
{
	auto endIt = pixelBuffer.end();
	auto it = pixelBuffer.begin();
	while (it != endIt)
	{
		*it = 0x00ffffff - *it;
		++it;
	}
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

int Renderer::getNyquist() const
{
	return nyquist;
}

void Renderer::setNyquist(int value)
{
	nyquist = value;
}

int Renderer::getFreqStep() const
{
	return freqStep;
}

void Renderer::setFreqStep(int value)
{
	freqStep = value;
}

int Renderer::getNumTimeDivs() const
{
	return numTimeDivs;
}

void Renderer::setNumTimeDivs(int value)
{
	numTimeDivs = value;
}

double Renderer::getStartTime() const
{
	return startTime;
}

void Renderer::setStartTime(double value)
{
	startTime = value;
}

double Renderer::getFinishTime() const
{
	return finishTime;
}

void Renderer::setFinishTime(double value)
{
	finishTime = value;
}

std::string Renderer::getInputFilename() const
{
	return inputFilename;
}

void Renderer::setInputFilename(const std::string &value)
{
	inputFilename = value;
}

std::string Renderer::getTitle() const
{
	return title;
}

void Renderer::setTitle(const std::string &value)
{
	title = value;
}

std::string Renderer::getHorizAxisLabel() const
{
	return horizAxisLabel;
}

void Renderer::setHorizAxisLabel(const std::string &value)
{
	horizAxisLabel = value;
}

std::string Renderer::getVertAxisLabel() const
{
	return vertAxisLabel;
}

std::vector<uint32_t> Renderer::getPixelBuffer() const
{
	return pixelBuffer;
}

double Renderer::getDynRange() const
{
	return dynRange;
}

void Renderer::setDynRange(double value)
{
	dynRange = value;
}

void Renderer::setVertAxisLabel(const std::string &value)
{
	vertAxisLabel = value;
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

void Renderer::setMargins()
{
	// estimate width of left margin
	cairo_text_extents_t hmLabelTextExtents;
	cairo_set_font_size(cr, fontSizeNormal);
	cairo_text_extents(cr, "- xxxx", &hmLabelTextExtents);
	marginLeft = hmOriginX + hmWidth + tickWidth + hmLabelTextExtents.x_advance;

	// estimate width of right margin
	cairo_text_extents_t fLabelTextExtents;
	cairo_set_font_size(cr, fontSizeNormal);
	cairo_text_extents(cr, "999999", &fLabelTextExtents);
	marginRight = 1.5 * tickWidth + fLabelTextExtents.x_advance + 2.5 * fLabelTextExtents.height;

	// estimate height of top margin
	cairo_set_font_size(cr, fontSizeHeading);
	cairo_text_extents_t titleTextExtents;
	cairo_text_extents(cr, "Spectrogram", &titleTextExtents);
	cairo_set_font_size(cr, fontSizeNormal);
	cairo_text_extents_t infoTextExtents;
	cairo_text_extents(cr, "XXX", &infoTextExtents);
	marginTop = 2.0 * (titleTextExtents.height + infoTextExtents.height);

	// estimate height of bottom margin
	cairo_text_extents_t timeLabelTextExtents;
	cairo_text_extents(cr, "1.0", &timeLabelTextExtents);
	cairo_text_extents_t horizAxisLabelTextExtents;
	cairo_text_extents(cr, "Tims(s)", &horizAxisLabelTextExtents);
	marginBottom = 2.5 * (timeLabelTextExtents.height + horizAxisLabelTextExtents.height);
}

} // namespace Sndspec
