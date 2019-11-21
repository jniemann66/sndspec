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

	void renderSpectrogram(const Parameters& parameters, const SpectrogramResults<double>& spectrogramData);
	void renderSpectrum(const Parameters& parameters, const std::vector<std::vector<double> > spectrumData);
	void makeNegativeImage();
	bool writeToFile(const std::string &filename);
	void clear();

	// setters
	void setHeatMapPalette(const std::vector<int32_t> &value);
	void setNyquist(int value); // required for frequency axis
	void setFreqStep(int value); // required for frequency axis
	void setNumTimeDivs(int value); // required for time axis
	void setStartTime(double value); // required for time axis
	void setFinishTime(double value); // required for time axis
	void setInputFilename(const std::string &value); // required to display input filename
	void setDynRange(double value); // required for showing heatmap dB values
	void setTitle(const std::string &value);
	void setHorizAxisLabel(const std::string &value);
	void setVertAxisLabel(const std::string &value);

	// getters
	std::vector<int32_t> getHeatMapPalette() const;
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
	double getDynRange() const;
	std::vector<uint32_t> getPixelBuffer() const;


private:
	void drawBorder();

	void drawSpectrogramGrid();
	void drawSpectrogramTickmarks();
	void drawSpectrogramText();
	void drawSpectrogramHeatMap();

	void drawSpectrumGrid();
	void drawSpectrumTickmarks();
	void drawSpectrumText();

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
	std::string windowFunctionLabel;
	bool showWindowFunctionLabel{false};
	double dynRange;

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

	// (experimental)
//	std::vector<int32_t> heatMapPalette {
//		0x00440154,
//		0x00481567,
//		0x00482677,
//		0x00453781,
//		0x00404788,
//		0x0039568C,
//		0x0033638D,
//		0x002D708E,
//		0x00287D8E,
//		0x00238A8D,
//		0x001F968B,
//		0x0020A387,
//		0x0029AF7F,
//		0x003CBB75,
//		0x0055C667,
//		0x0073D055,
//		0x0095D840,
//		0x00B8DE29,
//		0x00DCE319,
//		0x00FDE725
//	};

	void setMargins();


};

} // namespace Sndspec

#endif // RENDERER_H
