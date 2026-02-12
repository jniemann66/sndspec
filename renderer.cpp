/*
* Copyright (C) 2019 - 2026 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#include "renderer.h"

#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cmath>

#include <iostream>

namespace Sndspec {

Renderer::Renderer(int width, int height)
	: width(width), height(height), pixelBuffer(static_cast<size_t>(width * height), 0)
{
	// set up cairo surface
	const cairo_format_t cairoFormat =  CAIRO_FORMAT_RGB24;
	int stride =  cairo_format_stride_for_width(cairoFormat, width);
	stride32 = stride / static_cast<int>(sizeof(uint32_t));
	surface = cairo_image_surface_create_for_data(reinterpret_cast<unsigned char*>(pixelBuffer.data()), cairoFormat, width, height, stride);
	cr = cairo_create(surface);

	// calculate dimensions of actual plot area
	setMargins();
	plotWidth = static_cast<int>(width - marginLeft - marginRight);
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

void Renderer::renderSpectrogram(const Parameters &parameters, const SpectrogramResults<double> &spectrogramData)
{
	int numChannels = spectrogramData.size();
	resolveEnabledChannels(parameters, numChannels);

	int numSpectrums = spectrogramData.at(0).size();
	int numBins = spectrogramData.at(0).at(0).size();

	int h = plotHeight - 2;
	double colorScale = heatMapPalette.size() / -parameters.getDynRange();
	int lastColorIndex = std::max(0, static_cast<int>(heatMapPalette.size()) - 1);
	windowFunctionLabel = "Window: " + parameters.getWindowFunctionDisplayName();
	showWindowFunctionLabel = parameters.getShowWindowFunctionLabel();

	for (int c = 0; c < numChannels; c++) {
		if (channelsEnabled.at(c)) {
			// plot just one, then break
			for (int y = 0; y < numBins; y++) {
				int lineAddr = plotOriginX + (plotOriginY + h - y) * stride32;
				for (int x = 0; x < numSpectrums; x++) {
					int colorindex = static_cast<int>(spectrogramData[c][x][y] * colorScale);
					int32_t color = heatMapPalette[std::max(0, std::min(colorindex, lastColorIndex))];
					pixelBuffer[x + lineAddr] = color;
				}
			}
			break;
		}
	}

	drawSpectrogramGrid();
	drawBorder();
	drawSpectrogramTickmarks();
	drawSpectrogramText();
	drawSpectrogramHeatMap(parameters.getLinearMag());
}

std::pair <std::vector<std::vector<double>>, double> Renderer::renderSpectrum(const Parameters &parameters, const std::vector<std::vector<double>>& spectrumData)
{
	const int numChannels = spectrumData.size();
	const int numBins =  spectrumData.at(0).size();

	resolveEnabledChannels(parameters, numChannels);
	showWindowFunctionLabel = parameters.getShowWindowFunctionLabel();
	windowFunctionLabel = "Window: " + parameters.getWindowFunctionDisplayName();

	// todo: move smoothing out of here and into it's own dedicated function - also try other filters like savitsky-golay etc
	const SpectrumSmoothingMode spectrumSmoothingMode = parameters.getSpectrumSmoothingMode();

	// these next 3 are only used for moving average
	const int N = std::max(1, numBins / plotWidth); // size of smoothing filter (not used for "peak" or "none")
	const double gd = (N - 1) / 2.0; // spatial domain (freq bins) compensation due to group delay from smoothing filter
	const double magScaling = 1.0 / N;

	// positioning and scaling constants
	constexpr double hTrim = -0.5; // horizontal centering tweak to position plot nicely on top of gridlines
	const double plotOriginX_ = plotOriginX + hTrim;
	const double hScaling = static_cast<double>(plotWidth) / numBins;
	const double vScaling = static_cast<double>(plotHeight) / parameters.getDynRange();

	// clip the plotting region
	cairo_rectangle(cr, plotOriginX_, plotOriginY, plotWidth, plotHeight);
	cairo_clip(cr);
	const double opacity = 0.8;

	// retval : buffer to contain final plotted y-coordinates
	std::vector<std::vector<double>> results{spectrumData.size(), std::vector<double>(numBins, 0.0)};

	for (int c = 0; c < numChannels; c++) {

		// if channel is disabled, skip this channel
		if (!channelsEnabled.at(c)) {
			continue;
		}

		cairo_set_line_width (cr, 1.0);
		Rgb chColor = spectrumChannelColors[std::min(static_cast<int>(spectrumChannelColors.size() - 1), c)];
		cairo_set_source_rgba(cr, chColor.red, chColor.green, chColor.blue, opacity);
		cairo_move_to(cr, plotOriginX_, plotOriginY - vScaling * spectrumData.at(c).at(0));

		if (spectrumSmoothingMode == None) {
			for (int i = 0; i < numBins; i++) {
				const double mag = spectrumData.at(c).at(i);
				const double y = plotOriginY - vScaling * mag;
				results[c][i] = mag;
				cairo_line_to(cr, plotOriginX_ + hScaling * i, y);
			}
		} else if (spectrumSmoothingMode == MovingAverage) {
			const int d = std::ceil(gd);
			double acc = 0.0;
			for (int i = 0; i < N; i++) {
				acc += spectrumData.at(c).at(i);
			}

			for (int i = N; i < numBins; i++) {
				acc += spectrumData.at(c).at(i);
				acc -= spectrumData.at(c).at(i - N);
				const double mag = magScaling * acc;
				const double y = plotOriginY - vScaling * mag;
				results[c][i - d] = mag;
				cairo_line_to(cr, plotOriginX_ + hScaling * (i - gd), y);
			}
		} else if (spectrumSmoothingMode == Peak) {
			for (int i = 0; i < N; i++) {
				double maxDB(-300);
				const double mag = spectrumData.at(c).at(i);
				for (int a = 0; a <= i; a++) {
					maxDB = std::max(maxDB, spectrumData.at(c).at(a));
				}
				const double y = plotOriginY - vScaling * maxDB;
				results[c][i] = mag;
				cairo_line_to(cr, plotOriginX_ + hScaling * i, y);
			}

			for (int i = N; i < numBins; i++) {
				const double mag = spectrumData.at(c).at(i);
				double maxDB(-300);
				for (int a = i - N + 1; a <= i; a++) {
					maxDB = std::max(maxDB, spectrumData.at(c).at(a));
				}
				const double y = plotOriginY - vScaling * maxDB;
				results[c][i] = mag;
				cairo_line_to(cr, plotOriginX_ + hScaling * i, y);
			}

		}
		cairo_stroke(cr);
	}

	cairo_reset_clip(cr);

	drawBorder();
	drawSpectrumGrid();
	drawSpectrumTickmarks(parameters.getLinearMag());
	drawSpectrumText();

	return {results, vScaling};
}

void Renderer::renderWindowFunction(const Parameters& parameters, const std::vector<double>& data)
{
	// positioning and scaling constants
	const double hScaling = static_cast<double>(plotWidth) / data.size();

	const bool freqDomain = !parameters.getPlotTimeDomain();
	const double hTrim = freqDomain ? - 0.0 : 0.5; // horizontal centering tweak to position plot nicely on top of gridlines
	const double vTrim = freqDomain ? 0.0 : 0.0;

	// freq-domain: start at centre (f=0)
	// time domain: left-to-right
	const double plotOriginX_ = freqDomain ? plotOriginX + hTrim + plotWidth / 2.0
										   : plotOriginX + hTrim;

	// freq domain: range is expected from 0dB .. -dB
	// time domain: range is expected to be from 0..1
	const double plotOriginY_ = freqDomain ? plotOriginY + vTrim
										   : plotOriginY + vTrim + plotHeight;

	const double vScaling = freqDomain ? - static_cast<double>(plotHeight) / parameters.getDynRange()
									   : - static_cast<double>(plotHeight - 1.0);

	if (freqDomain) {
		// clip the plotting region
		cairo_rectangle(cr, plotOriginX, plotOriginY, plotWidth, plotHeight);
		cairo_clip(cr);
	}

	const double opacity = 0.8;

	cairo_set_line_width (cr, 1.0);
	Rgb chColor = spectrumChannelColors[3];
	cairo_set_source_rgba(cr, chColor.red, chColor.green, chColor.blue, opacity);
	cairo_move_to(cr, plotOriginX_, plotOriginY - vScaling * data.at(0));

	for (int i = 0; i < data.size(); i++) {
		const double mag = data.at(i);
		const double x = plotOriginX_ + hScaling * i;
		const double y = plotOriginY_ + vScaling * mag;
		if (freqDomain) {
			const double x2 = plotOriginX_ - hScaling * i;
			const double y2 = y + plotHeight;
			// draw mirror-image (left of center)
			cairo_move_to(cr, x2, y2);
			cairo_line_to(cr, x2, y);
			// move to right-of centre
			cairo_move_to(cr, x, y2);

		}
		cairo_line_to(cr, x, y);
	}

	cairo_stroke(cr);

	cairo_reset_clip(cr);

	drawBorder();
	drawSpectrumGrid();
	drawSpectrumTickmarks(parameters.getLinearMag());
	drawSpectrumText();
}

// note: in Normal mode, channels may be disabled due to the following:
// 1. no signal present
// 2. user requested that the channel be omitted
// in Sum / Difference mode, channel zero is always enabled, and the others should be disabled

void Renderer::resolveEnabledChannels(const Parameters &parameters, int numChannels)
{
	if (channelsEnabled.empty()) {
		channelsEnabled.resize(numChannels, true);
	}

	switch (parameters.getChannelMode())
	{
	case Sum:
		channelMode = "Sum";
		channelsEnabled[0] = true;
		for (int ch = 1; ch < static_cast<int>(channelsEnabled.size()); ch++) {
			channelsEnabled[ch] = false;
		}
		break;
	case Difference:
		channelMode = "Difference";
		channelsEnabled[0] = true;
		for (int ch = 1; ch < static_cast<int>(channelsEnabled.size()); ch++) {
			channelsEnabled[ch] = false;
		}
		break;
	case Normal:
		channelMode = "Normal";
		auto requestedChannels = parameters.getSelectedChannels();
		if (!requestedChannels.empty()) {
			for (int ch = 0; ch < numChannels; ch++) {
				channelsEnabled[ch] = channelsEnabled.at(ch) && (requestedChannels.find(ch) != requestedChannels.end());
			}
		}
		break;
	}
}

void Renderer::drawSpectrogramGrid()
{
	const double opacity = 0.5;
	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, opacity);

	double yStep = plotHeight * static_cast<double>(freqStep) / nyquist;
	double y = plotOriginY + plotHeight - 1 ;

	while (y > plotOriginY) {
		cairo_move_to(cr, plotOriginX, y);
		cairo_line_to(cr, plotOriginX + plotWidth - 1, y);
		y -= yStep;
	}

	double fWidth = static_cast<double>(plotWidth);
	double xStep = fWidth / numTimeDivs;
	double x = plotOriginX;

	while (x < fWidth) {
		cairo_move_to(cr, x, plotOriginY);
		cairo_line_to(cr, x, plotOriginY + plotHeight - 1);
		x += xStep;
	}

	cairo_stroke (cr);
}

void Renderer::drawSpectrumGrid()
{
	const double opacity = 0.5;
	double yScale = plotHeight / dynRange;
	double yStep = yScale * 10;
	double y = plotOriginY + plotHeight - 1 ;
	double dashpattern[] = {4.0, 2.0};
	cairo_set_dash(cr, dashpattern, 2, 0.0);

	// draw horizontal gridlines
	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, opacity);
	while (y > plotOriginY) {
		cairo_move_to(cr, plotOriginX, y);
		cairo_line_to(cr, plotOriginX + plotWidth - 1, y);
		y -= yStep;
	}
	cairo_stroke (cr);

	double fWidth = static_cast<double>(plotWidth);
	double xStep = fWidth * freqStep / nyquist;
	double x = plotOriginX;
	double xf = plotOriginX + fWidth;

	// draw vertical gridlines
	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, opacity);
	while (x < xf) {
		cairo_move_to(cr, x, plotOriginY);
		cairo_line_to(cr, x, plotOriginY + plotHeight - 1);
		x += xStep;
	}

	cairo_stroke (cr);
	cairo_set_dash(cr, dashpattern, 0, 0.0);
}

void Renderer::drawSpectrumTickmarks(bool linearMag)
{
	const int s = 10;
	constexpr int fx = s + 5;
	const int fy = 4;

	const double yScale = plotHeight / dynRange;
	const double yStep = yScale * 10;
	double y = plotOriginY + plotHeight - 1;

	// draw dB tickmarks and labels
	cairo_set_line_width (cr, 2);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	char dbLabelBuf[20];
	double dB = dynRange;
	while (y > plotOriginY) {
		if (linearMag) {
			sprintf(dbLabelBuf, "%d", static_cast<int>(100.0-dB));
		} else {
			sprintf(dbLabelBuf, "%d", static_cast<int>(-dB));
		}
		cairo_move_to(cr, plotOriginX + plotWidth, y);
		cairo_line_to(cr, plotOriginX + s + plotWidth - 1, y);
		cairo_move_to(cr, plotOriginX + fx + plotWidth - 1, y + fy);
		cairo_show_text(cr, dbLabelBuf);
		y -= yStep;
		dB -= 10;
	}
	cairo_stroke (cr);

	double fWidth = static_cast<double>(plotWidth);
	double xStep = fWidth * freqStep / nyquist;
	char fLabelBuf[20];
	constexpr int ty = s + 15;
	double f = 0.0;
	double x = plotOriginX;
	while (x < (fWidth + plotOriginX)) {
		sprintf(fLabelBuf, "%6.0f", f);
		cairo_text_extents_t freqLabelTextExtents;
		cairo_text_extents(cr, fLabelBuf, &freqLabelTextExtents);
		cairo_move_to(cr, x, plotOriginY + plotHeight);
		cairo_line_to(cr, x, plotOriginY + plotHeight + s - 1);
		cairo_move_to(cr, x - freqLabelTextExtents.x_advance * 0.5, plotOriginY + plotHeight + ty - 1);
		cairo_show_text(cr, fLabelBuf);
		x += xStep;
		f += freqStep;
	}

	cairo_stroke (cr);
}

void Renderer::drawSpectrumText()
{
	const double opacity = 0.8;
	double s = 20.0;

	// heading
	cairo_text_extents_t headingTextExtents;
	cairo_set_font_size(cr, 16);
	cairo_text_extents(cr, title.c_str(), &headingTextExtents);
	cairo_move_to(cr, plotOriginX + (plotWidth - headingTextExtents.x_advance) / 2.0, s); // place at center of plot area
	cairo_show_text(cr, title.c_str());
	cairo_set_font_size(cr, 13);

	// info
	std::string infoString = " ";
	if (startTime == finishTime && std::fpclassify(startTime) == FP_ZERO) {
		infoString = inputFilename;
	} else {
		infoString = inputFilename + " " + formatTimeRange(startTime, finishTime);
	}

	cairo_text_extents_t infoExtents;
	cairo_text_extents(cr, infoString.c_str(), &infoExtents);
	cairo_move_to(cr, plotOriginX, plotOriginY - infoExtents.height);
	cairo_show_text(cr, infoString.c_str());

	// channel mode
	if (channelMode == "Normal") {
		int xpos = plotOriginX + plotWidth;
		for (int ch = channelsEnabled.size() - 1; ch >=  0; ch--) {
			Rgb chColor = spectrumChannelColors[std::min(static_cast<int>(spectrumChannelColors.size() - 1), ch)];
			cairo_set_source_rgba(cr, chColor.red, chColor.green, chColor.blue, opacity);
			if (channelsEnabled.at(ch)) {
				cairo_text_extents_t extents;
				std::string s;
				if (channelsEnabled.size() == 1) {
					s = "";
				} else if (channelsEnabled.size() == 2) {
					s = (ch == 1) ? " R" : " L"; // stereo
				} else {
					s = " " + std::to_string(ch);
				}
				cairo_text_extents(cr, s.c_str(), &extents);
				xpos -= extents.x_advance;
				cairo_move_to(cr, xpos, height - extents.height);
				cairo_show_text(cr, s.c_str());
			}
		}
	} else {
		Rgb chColor = spectrumChannelColors.at(0);
		cairo_set_source_rgba(cr, chColor.red, chColor.green, chColor.blue, opacity);
		cairo_text_extents_t chModeExtents;
		cairo_text_extents(cr, channelMode.c_str(), &chModeExtents);
		cairo_move_to(cr, plotOriginX + plotWidth - chModeExtents.x_advance, height - infoExtents.height);
		cairo_show_text(cr, channelMode.c_str());
	}

	// resume white color
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

	// window function label
	if (showWindowFunctionLabel) {
		cairo_text_extents_t windowFuncExtents;
		cairo_text_extents(cr, windowFunctionLabel.c_str(), &windowFuncExtents);
		cairo_move_to(cr, plotOriginX + plotWidth - windowFuncExtents.x_advance, plotOriginY - infoExtents.height);
		cairo_show_text(cr, windowFunctionLabel.c_str());
	}

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

Renderer::LocalPeakData_t Renderer::getRankedLocalMaxima(const std::vector<double>& data)
{
	const size_t thresh = 10;

	Renderer::LocalPeakData_t results; // retval
	if (data.size() >= 3) { // need at least 3 elements
		// Iterate over all elements except first and last
		for (size_t i = 1; i < data.size() - 1; ++i) {
			if (data[i] > data[i - 1] && data[i] > data[i + 1]) {
				// Found a local maximum
				results[data[i]] = i;
			}
		}
	}

	return results;
}

std::vector<Marker> Renderer::getTopNFrequencyMarkers(const Parameters& parameters, const std::vector<double>& data, double vScaling, size_t n, int channel)
{
	const auto rankedFrequencies = getRankedLocalMaxima(data);

	const size_t numBins = data.size();
	const double hScaling = static_cast<double>(plotWidth) / numBins;

	std::vector<Marker> markers; // retval

	const double thresh_Hz = parameters.getTopN_minSpacing().value_or(10.0);
	size_t thresh = std::ceil(thresh_Hz / (static_cast<double>(nyquist) / numBins));
	constexpr bool verbose = false;

	if constexpr (verbose) {
		std::cout << "bins=" << numBins << " Hz/bin= " << nyquist / numBins << " thresh(bins)= " << thresh << std::endl;
	}

	size_t i = 0;

	std::set<size_t> used_indexes;

	for (const auto& [mag, freqIndex] : rankedFrequencies) {
		if ((i + 1) > n) {
			break;
		}

		Marker m;
		m.index = i;
		m.freq = nyquist * static_cast<double>(freqIndex) /  (numBins - 1);
		m.visible = true;
		m.color = spectrumChannelColors[std::min(static_cast<int>(spectrumChannelColors.size() - 1), channel)];
		m.mag = mag;
		m.x = plotOriginX + hScaling * freqIndex;
		m.y = plotOriginY - vScaling * mag;

		// skip if peak is too close to another
		if (auto it = used_indexes.lower_bound(freqIndex - thresh); it != used_indexes.end() && *it <= freqIndex + thresh) {
			if (verbose) {
				std::cout << "peak frequency " << m.freq << "Hz (mag=" << m.mag << ") is too close to previous peak at "
						  << nyquist * static_cast<double>(*it) / numBins << std::endl;
			}
			continue; // too close to another peak
		}

		if constexpr (verbose) {
			std::cout << "index=" << i << " Peak: frequency=" << m.freq <<" mag=" << m.mag << std::endl;
		}

		used_indexes.insert(freqIndex);
		markers.push_back(m);
		++i;
	}

	return markers;
}

void Renderer::drawMarkers(const std::vector<Marker>& markers)
{
	constexpr double marker_width = 5.0; // maker base width
	constexpr double marker_voffset = 2.0; // vertical distance of marker tip from actual point of interest
	constexpr double marker_height = 10.0; // marker tick height (including voffset)
	constexpr double label_voffset = 3.0; // vertical distance between marker base and bottom of text
	constexpr double marker_halfwidth = marker_width / 2;

	cairo_set_line_width (cr, 1.5);
	cairo_set_font_size(cr, 13);

	for (const Marker& m : markers) {

		// set colour
		cairo_set_source_rgb(cr, m.color.red, m.color.green, m.color.blue);

		// draw triangle marker shape
		const double& tip_x = m.x;
		const double tip_y = m.y - marker_voffset;
		cairo_move_to(cr, tip_x, tip_y);
		cairo_line_to(cr, tip_x - marker_halfwidth, m.y - marker_height);
		cairo_line_to(cr, tip_x + marker_halfwidth, m.y - marker_height);
		cairo_line_to(cr, tip_x, tip_y);

		// fill marker, if that floats your boat
		constexpr bool fillMarkers = false;
		if constexpr (fillMarkers) {
			cairo_fill(cr);
		}

		// prepare label measurements
		const std::string txt = m.displayText();
		cairo_text_extents_t markerExtents;
		cairo_text_extents(cr, txt.c_str(), &markerExtents);
		const double txt_x = m.x - markerExtents.x_advance / 2.0;
		const double txt_y = m.y - marker_height - label_voffset;

		// check if label will clash with other stuff at the top of the plot
		if (tip_y - marker_height - markerExtents.height < plotOriginY) {
			cairo_stroke(cr);
			// Clear rectangle underneath
			cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
			constexpr double margin = 1.5;
			cairo_rectangle(cr, txt_x - margin, txt_y + margin, markerExtents.width + 2 * margin, -markerExtents.height - 2 * margin);
			cairo_fill(cr);
			cairo_stroke(cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
		}

		// draw the text
		cairo_move_to(cr, txt_x, txt_y);
		cairo_show_text(cr, txt.c_str());
	}

	cairo_stroke(cr);
}

std::vector<bool> Renderer::getChannelsEnabled() const
{
	return channelsEnabled;
}

void Renderer::setChannelsEnabled(const std::vector<bool> &value)
{
	channelsEnabled = value;
}


void Renderer::drawBorder()
{
	cairo_set_line_width (cr, 2);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	double s = 1;
	cairo_move_to(cr, plotOriginX, plotOriginY + s);
	cairo_line_to(cr, plotOriginX + plotWidth - s, plotOriginY + s);
	cairo_line_to(cr, plotOriginX + plotWidth - s, plotOriginY + plotHeight - s);
	cairo_line_to(cr, plotOriginX,  plotOriginY + plotHeight - s);
	cairo_close_path(cr);

	cairo_stroke(cr);
}

void Renderer::drawSpectrogramTickmarks()
{
	cairo_set_line_width (cr, 2);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

	const int s = 10;
	constexpr int fx = s + 5;
	const int fy = 4;

	cairo_set_font_size(cr, 13);

	double yStep = plotHeight * static_cast<double>(freqStep) / nyquist;
	double y = plotOriginY + plotHeight - 1 ;
	int f = 0;

	char fLabelBuf[20];

	while (y > plotOriginY) {
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
	double t = startTime;
	const int tx = -5;
	constexpr int ty = s + 15;

	while (x < fWidth) {
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

void Renderer::drawSpectrogramText()
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

	// window function label
	if (showWindowFunctionLabel) {
		cairo_text_extents_t windowFuncExtents;
		cairo_text_extents(cr, windowFunctionLabel.c_str(), &windowFuncExtents);
		cairo_move_to(cr, plotOriginX + plotWidth - windowFuncExtents.x_advance, plotOriginY - infoExtents.height);
		cairo_show_text(cr, windowFunctionLabel.c_str());
	}

	// horizAxis
	cairo_text_extents_t horizAxisLabelExtents;
	cairo_text_extents(cr, horizAxisLabel.c_str(), &horizAxisLabelExtents);
	cairo_move_to(cr, plotOriginX + (plotWidth - horizAxisLabelExtents.x_advance) / 2.0, height - horizAxisLabelExtents.height);
	cairo_show_text(cr, horizAxisLabel.c_str());

	// channel mode
	if (channelMode == "Normal") {
		int xpos = plotOriginX + plotWidth;
		for (int ch = channelsEnabled.size() - 1; ch >=  0; ch--) {
			if (channelsEnabled.at(ch)) {
				cairo_text_extents_t extents;
				std::string s;
				if (channelsEnabled.size() == 1) {
					s = "";
				} else if (channelsEnabled.size() == 2) {
					s = (ch == 1) ? " R" : " L"; // stereo
				} else {
					s = " " + std::to_string(ch);
				}
				cairo_text_extents(cr, s.c_str(), &extents);
				xpos -= extents.x_advance;
				cairo_move_to(cr, xpos, height - extents.height);
				cairo_show_text(cr, s.c_str());
				break;
			}
		}
	} else {
		cairo_text_extents_t chModeExtents;
		cairo_text_extents(cr, channelMode.c_str(), &chModeExtents);
		cairo_move_to(cr, plotOriginX + plotWidth - chModeExtents.x_advance, height - infoExtents.height);
		cairo_show_text(cr, channelMode.c_str());
	}

	// vertAxis
	cairo_text_extents_t vertAxisLabelExtents;
	cairo_text_extents(cr, vertAxisLabel.c_str(), &vertAxisLabelExtents);
	cairo_save(cr);
	cairo_move_to(cr, width - s, plotOriginY + (plotHeight) / 2.0);
	cairo_rotate(cr, M_PI_2);
	cairo_show_text(cr, vertAxisLabel.c_str());
	cairo_restore(cr);
}

void Renderer::drawSpectrogramHeatMap(bool linearMag)
{
	double sc = static_cast<double>(heatMapPalette.size()) / plotHeight;

	// draw the heatmap colours
	for (int y = 0; y < plotHeight; y++) {
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

	// draw the units heading
	cairo_text_extents_t dBExtents;
	if (linearMag) {
		cairo_text_extents(cr, "%", &dBExtents);
		cairo_move_to(cr, hmOriginX, plotOriginY - dBExtents.height);
		cairo_show_text(cr, "%");
	} else {
		cairo_text_extents(cr, "dB", &dBExtents);
		cairo_move_to(cr, hmOriginX, plotOriginY - dBExtents.height);
		cairo_show_text(cr, "dB");
	}

	// draw the dB tickmarks and labels
	double dB = 0.0;
	double dBsc = plotHeight / dynRange;
	double xa = hmOriginX + hmWidth + s;
	double xb = xa + tickWidth;
	char dbBuf[20];
	while (dB < dynRange) {
		if (linearMag) {
			sprintf(dbBuf, "%3.0f", 100.0 - dB);
		} else {
			sprintf(dbBuf, "%3.0f", -dB);
		}
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

double Renderer::getNyquist() const
{
	return nyquist;
}

void Renderer::setNyquist(double value)
{
	nyquist = value;
}

double Renderer::getFreqStep() const
{
	return freqStep;
}

void Renderer::setFreqStep(double value)
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

std::string Renderer::formatTimeRange(const double startSecs, const double finishSecs)
{
	int h0 = startSecs / 3600;
	int m0 = (startSecs - h0 * 3600) / 60;
	double s0 = startSecs - h0 * 3600 - m0 * 60;

	int h1 = finishSecs / 3600;
	int m1 = (finishSecs - h1 * 3600) / 60;
	double s1 = finishSecs - h1 * 3600 - m1 * 60;

	char buf[100];
	sprintf(buf, "%02d:%02d:%07.4f - %02d:%02d:%07.4f", h0, m0, s0, h1, m1, s1);
	return std::string{buf};
}

} // namespace Sndspec
