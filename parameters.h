/*
* Copyright (C) 2019 - 2026 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef OPTIONS_H
#define OPTIONS_H

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace Sndspec {

const std::vector<std::string> fileTypes
{
	".aifc",
	".aiff",
	".aif",
	".au",
	".avr",
	".caf",
	".flac",
	".htk",
	".iff",
	".mat",
	".mpc",
	".oga",
	".paf",
	".pvf",
	".rf64",
	".sd2",
	".sds",
	".sf",
	".voc",
	".w64",
	".wav",
	".wve",
	".xi"
};

enum OptionID
{
	Filenames,
	DynRange,
	OutputDir,
	Height,
	Width,
	TimeRange,
	WhiteBackground,
	WindowFunction,
	ShowWindows,
	SpectrumMode,
	Smoothing,
	Channel,
	LinearMag,
	FrequencyStep,
	PeakSelection,
	PlotWindowFunction,
	Recursive,
	Version,
	Help
};

struct Option
{
	OptionID optionID;
	std::string longOption;
	std::string shortOption;
	bool isMandatory;
	std::string description;
	std::vector<std::string> args;
};

enum SpectrumSmoothingMode
{
	None,
	MovingAverage,
	Peak
};

enum ChannelMode
{
	Normal,
	Sum,
	Difference
};

const std::vector<Option> options
{
	{OptionID::DynRange, "--dyn-range", "", false, "Set Dynamic Range in dB", {"n"}},
	{OptionID::OutputDir, "--output-dir", "-o", false, "Set Output directory", {"path"}},
	{OptionID::Height, "--height","-h", false, "Set Image Height in Pixels", {"n"}},
	{OptionID::Width, "--width", "-w", false, "Set Image Width in Pixels", {"n"}},
	{OptionID::TimeRange, "--time-range", "-t", false, "Set	Time Range in seconds", {"start-time", "finish-time"}},
	{OptionID::WhiteBackground, "--white-background", "", false, "White Background (instead of black) with inverted colors", {}},
	{OptionID::WindowFunction, "--window", "-W", false, "Set the window function", {"name"}},
	{OptionID::ShowWindows, "--show-windows", "", false, "Show a list of available window functions", {}},
	{OptionID::SpectrumMode, "--spectrum", "", false, "Plot a Spectrum instead of Spectrogram", {}},
	{OptionID::Smoothing, "--smoothing", "-S", false, "Set Spectrum Smoothing Mode (default:peak)", {"moving average|peak|none"}},
	{OptionID::Channel, "--channel", "-c", false, "select specific channels and set channel mode", {"[all|[L|R|0|1|2|...]...] [sum|difference|normal]"}},
	{OptionID::LinearMag, "--linear-mag", "-l", false, "Set magnitude scale to be linear", {}},
	{OptionID::FrequencyStep, "--frequency-step", "-f", false, "Set interval of frequency tick marks in Hz", {"n"}},
	{OptionID::PeakSelection, "--peak-selection", "-p", false, "Annotate the top n local peaks in the results", {"n [min-spacing(Hz)]"}},
	{OptionID::PlotWindowFunction, "--plot-window", "", false, "Plot window function", {"name [time domain]"}},
	{OptionID::Recursive, "--recursive", "-r", false, "Recursive directory traversal", {}},

#ifdef SNDSPEC_VERSION
	{OptionID::Version, "--version", "", false, "Show program version", {}},
#endif

	{OptionID::Help, "--help", "", false, "Help", {}}

};

class Parameters
{
public:
	Parameters() = default;
	std::string fromArgs(const std::vector<std::string>& args);
	std::string showHelp();
	std::string showWindowList();

	// setters
	void setInputFiles(const std::vector<std::string> &val);
	void setOutputPath(const std::string &val);
	void setImgWidth(int val);
	void setIngHeight(int val);
	void setDynRange(double val);
	void setHasTimeRange(bool val);
	void setHasWhiteBackground(bool val);
	void setStart(double val);
	void setFinish(double val);
	void setWindowFunction(const std::string &val);
	void setWindowFunctionDisplayName(const std::string &val);
	void setShowWindowFunctionLabel(bool val);
	void setShowWindows(bool val);
	void setSpectrumMode(bool val);
	void setSpectrumSmoothingMode(const SpectrumSmoothingMode &val);
	void setSelectedChannels(const std::set<int> &val);
	void setChannelMode(const ChannelMode &val);
	void setLinearMag(bool val);
	void setFrequencyStep(int val);
	void setTopN(std::optional<int> val);
	void setTopN_minSpacing(std::optional<double> newTopN_minSpacing);
	void setPlotWindowFunction(bool val);
	void setPlotTimeDomain(bool val);

	// getters
	std::vector<std::string> getInputFiles() const;
	std::string getOutputPath() const;
	int getImgWidth() const;
	int getImgHeight() const;
	double getDynRange() const;
	double getStart() const;
	double getFinish() const;
	bool hasTimeRange() const;
	bool hasWhiteBackground() const;
	std::string getWindowFunction() const;
	std::string getWindowFunctionDisplayName() const;
	bool getShowWindows() const;
	bool getShowWindowFunctionLabel() const;	// const int numChannels = spectrumData.size();
	// const int numBins =  spectrumData.at(0).size();

	// resolveEnabledChannels(parameters, numChannels);
	// showWindowFunctionLabel = parameters.getShowWindowFunctionLabel();
	// windowFunctionLabel = "Window: " + parameters.getWindowFunctionDisplayName();

	// // todo: move smoothing out of here and into it's own dedicated function - also try other filters like savitsky-golay etc
	// const SpectrumSmoothingMode spectrumSmoothingMode = parameters.getSpectrumSmoothingMode();

	// // these next 3 are only used for moving average
	// const int N = std::max(1, numBins / plotWidth); // size of smoothing filter (not used for "peak" or "none")
	// const double gd = (N - 1) / 2.0; // spatial domain (freq bins) compensation due to group delay from smoothing filter
	// const double magScaling = 1.0 / N;

	// // positioning and scaling constants
	// constexpr double hTrim = -0.5; // horizontal centering tweak to position plot nicely on top of gridlines
	// const double plotOriginX_ = plotOriginX + hTrim;
	// const double hScaling = static_cast<double>(plotWidth) / numBins;
	// const double vScaling = static_cast<double>(plotHeight) / parameters.getDynRange();

	// // clip the plotting region
	// cairo_rectangle(cr, plotOriginX_, plotOriginY, plotWidth, plotHeight);
	// cairo_clip(cr);
	// const double opacity = 0.8;

	// // retval : buffer to contain final plotted y-coordinates
	// std::vector<std::vector<double>> results{spectrumData.size(), std::vector<double>(numBins, 0.0)};

	// for (int c = 0; c < numChannels; c++) {

	// 	// if channel is disabled, skip this channel
	// 	if (!channelsEnabled.at(c)) {
	// 		continue;
	// 	}

	// 	cairo_set_line_width (cr, 1.0);
	// 	Rgb chColor = spectrumChannelColors[std::min(static_cast<int>(spectrumChannelColors.size() - 1), c)];
	// 	cairo_set_source_rgba(cr, chColor.red, chColor.green, chColor.blue, opacity);
	// 	cairo_move_to(cr, plotOriginX_, plotOriginY - vScaling * spectrumData.at(c).at(0));

	// 	if (spectrumSmoothingMode == None) {
	// 		for (int i = 0; i < numBins; i++) {
	// 			const double mag = spectrumData.at(c).at(i);
	// 			const double y = plotOriginY - vScaling * mag;
	// 			results[c][i] = mag;
	// 			cairo_line_to(cr, plotOriginX_ + hScaling * i, y);
	// 		}
	// 	} else if (spectrumSmoothingMode == MovingAverage) {
	// 		const int d = std::ceil(gd);
	// 		double acc = 0.0;
	// 		for (int i = 0; i < N; i++) {
	// 			acc += spectrumData.at(c).at(i);
	// 		}

	// 		for (int i = N; i < numBins; i++) {
	// 			acc += spectrumData.at(c).at(i);
	// 			acc -= spectrumData.at(c).at(i - N);
	// 			const double mag = magScaling * acc;
	// 			const double y = plotOriginY - vScaling * mag;
	// 			results[c][i - d] = mag;
	// 			cairo_line_to(cr, plotOriginX_ + hScaling * (i - gd), y);
	// 		}
	// 	} else if (spectrumSmoothingMode == Peak) {
	// 		for (int i = 0; i < N; i++) {
	// 			double maxDB(-300);
	// 			const double mag = spectrumData.at(c).at(i);
	// 			for (int a = 0; a <= i; a++) {
	// 				maxDB = std::max(maxDB, spectrumData.at(c).at(a));
	// 			}
	// 			const double y = plotOriginY - vScaling * maxDB;
	// 			results[c][i] = mag;
	// 			cairo_line_to(cr, plotOriginX_ + hScaling * i, y);
	// 		}

	// 		for (int i = N; i < numBins; i++) {
	// 			const double mag = spectrumData.at(c).at(i);
	// 			double maxDB(-300);
	// 			for (int a = i - N + 1; a <= i; a++) {
	// 				maxDB = std::max(maxDB, spectrumData.at(c).at(a));
	// 			}
	// 			const double y = plotOriginY - vScaling * maxDB;
	// 			results[c][i] = mag;
	// 			cairo_line_to(cr, plotOriginX_ + hScaling * i, y);
	// 		}

	// 	}
	// 	cairo_stroke(cr);
	// }

	// cairo_reset_clip(cr);

	// drawBorder();
	// drawSpectrumGrid();
	// drawSpectrumTickmarks(parameters.getLinearMag());
	// drawSpectrumText();

	// return {results, vScaling};
	bool getSpectrumMode() const;
	SpectrumSmoothingMode getSpectrumSmoothingMode() const;
	std::set<int> getSelectedChannels() const;
	ChannelMode getChannelMode() const;
	bool getLinearMag() const;
	int getFrequencyStep() const;
	std::optional<int> getTopN() const;
	std::optional<double> getTopN_minSpacing() const;
	bool getPlotWindowFunction() const;
	bool getPlotTimeDomain() const;


private:
	std::vector<std::string> inputFiles;
	std::string outputPath;
	int imgWidth{1024};
	int imgHeight{768};
	double dynRange{190};
	bool timeRange{false};
	double start{0.0};
	double finish{0.0};
	bool whiteBackground{false};
	std::string windowFunction{"kaiser"};
	std::string windowFunctionDisplayName{"Kaiser"};
	bool showWindowFunctionLabel{false}; // flag to show the name of window function on the rendered output
	bool showWindows{false}; // flag to provide a list of available window functions
	bool spectrumMode{false};
	bool plotWindowFunction{false};
	bool plotTimeDomain{false};
	SpectrumSmoothingMode spectrumSmoothingMode{Peak};
	std::set<int> selectedChannels; // if the set is empty, it is interpreted as "all channels"
	ChannelMode channelMode{Normal};
	bool linearMag{false};
	int frequencyStep{5000};
	std::optional<int> topN;
	std::optional<double> topN_minSpacing;
	bool recursiveDirectoryTraversal{false};

	void processChannelArgs(const std::vector<std::string> &args);
};

} // namespace Options

#endif // OPTIONS_H
