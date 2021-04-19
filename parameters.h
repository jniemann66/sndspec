/*
* Copyright (C) 2019 - 2021 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef OPTIONS_H
#define OPTIONS_H

#include "fsdetect.h"

#include <string>
#include <vector>
#include <set>
#include <variant>

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

#ifdef FS_AVAILABLE
	Recursive,
#endif

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
	{OptionID::TimeRange, "--time-range", "-t", false, "Set Time Range in seconds", {"start-time", "finish-time"}},
	{OptionID::WhiteBackground, "--white-background", "", false, "White Background (instead of black) with inverted colors", {}},
	{OptionID::WindowFunction, "--window", "-W", false, "Set the window function", {"name"}},
	{OptionID::ShowWindows, "--show-windows", "", false, "Show a list of available window functions", {}},
	{OptionID::SpectrumMode, "--spectrum", "", false, "Plot a Spectrum instead of Spectrogram", {}},
	{OptionID::Smoothing, "--smoothing", "-S", false, "Set Spectrum Smoothing Mode (default:peak)", {"moving average|peak|none"}},
	{OptionID::Channel, "--channel", "-c", false, "select specific channels and set channel mode", {"[all|[L|R|0|1|2|...]...] [sum|difference|normal]"}},
	{OptionID::LinearMag, "--linear-mag", "-c", false, "Set magnitude scale to be linear", {}},

#ifdef FS_AVAILABLE
	{OptionID::Recursive, "--recursive", "-r", false, "Recursive directory traversal", {}},
#endif

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
	void setInputFiles(const std::vector<std::string> &value);
	void setOutputPath(const std::string &value);
	void setImgWidth(int value);
	void setIngHeight(int value);
	void setDynRange(double value);
	void setHasTimeRange(bool value);
	void setHasWhiteBackground(bool value);
	void setStart(double value);
	void setFinish(double value);
	void setWindowFunction(const std::string &value);
	void setWindowFunctionDisplayName(const std::string &value);
	void setShowWindowFunctionLabel(bool value);
	void setShowWindows(bool value);
	void setSpectrumMode(bool value);
	void setSpectrumSmoothingMode(const SpectrumSmoothingMode &value);
	void setSelectedChannels(const std::set<int> &value);
	void setChannelMode(const ChannelMode &value);
	void setLinearMag(bool value);

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
	bool getShowWindowFunctionLabel() const;
	bool getSpectrumMode() const;
	SpectrumSmoothingMode getSpectrumSmoothingMode() const;
	std::set<int> getSelectedChannels() const;
	ChannelMode getChannelMode() const;
	bool getLinearMag() const;

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
	SpectrumSmoothingMode spectrumSmoothingMode{Peak};
	std::set<int> selectedChannels; // if the set is empty, it is interpreted as "all channels"
	ChannelMode channelMode{Normal};
	bool linearMag{false};

#ifdef FS_AVAILABLE
	bool recursiveDirectoryTraversal{false};
#endif

	void processChannelArgs(const std::vector<std::string> &args);
};

} // namespace Options

#endif // OPTIONS_H
