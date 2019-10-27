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
	Renderer(int width, int height);
	~Renderer();

	void render(const Parameters& parameters, const SpectrogramResults<double>& spectrogramData);

	std::vector<int32_t> getHeatMapPalette() const;
	void setHeatMapPalette(const std::vector<int32_t> &value);
	bool writeToFile(const std::string &filename);
	void drawGrid();
	void drawBorder();
	void drawTickmarks();
	void drawText();
	void drawHeatMap(double dynRange);
	void makeNegativeImage();
	void clear();

	// setters
	void setNyquist(int value);
	void setFreqStep(int value);
	void setNumTimeDivs(int value);
	void setStartTime(double value);
	void setFinishTime(double value);
	void setInputFilename(const std::string &value);
	void setTitle(const std::string &value);
	void setHorizAxisLabel(const std::string &value);
	void setVertAxisLabel(const std::string &value);

	// getters
	int getPlotWidth() const;
	int getPlotHeight() const;
	int getNyquist() const;
	int getFreqStep() const;
	int getNumTimeDivs() const;
	double getStartTime() const;
	double getFinishTime() const;
	std::string getInputFilename() const;
	std::string getTitle() const;
	std::string getHorizAxisLabel() const;
	std::string getVertAxisLabel() const;

private:
	// dimensions of whole image
	int width;
	int height;

	// properties required for labelling the chart
	int nyquist;
	int freqStep;
	int numTimeDivs{5};
	double startTime;
	double finishTime;
	std::string title{"Spectrogram"};
	std::string inputFilename;
	std::string horizAxisLabel{"Time (s)"};
	std::string vertAxisLabel{"Frequency (Hz)"};

	// font sizes
	const double fontSizeNormal{13.0};
	const double fontSizeHeading{16.0};

	// heatmap origin
	const int hmOriginX{10};
	int hmOriginY; // depends on marginTop

	// heatmap width
	const int hmWidth{10};
	int hmLabelWidth;

	// plot origin
	int plotOriginX;
	int plotOriginY;

	// plot dimensions
	int plotWidth;
	int plotHeight;

	// tick Width
	const double tickWidth{10.0};

	// margins
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

	void setMargins();
};

} // namespace Sndspec

#endif // RENDERER_H
