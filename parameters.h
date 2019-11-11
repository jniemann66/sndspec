#ifndef OPTIONS_H
#define OPTIONS_H

#include "fsdetect.h"

#include <string>
#include <vector>
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

#ifdef FS_AVAILABLE
	Recursive,
#endif

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

const std::vector<Option> options
{
	{OptionID::DynRange, "--dyn-range", "", false, "Set Dynamic Range in dB", {"n"}},
	{OptionID::OutputDir, "--output-dir", "-o", false, "Set Output directory", {"path"}},
	{OptionID::Height, "--height","-h", false, "Set Image Height in Pixels", {"n"}},
	{OptionID::Width, "--width", "-w", false, "Set Image Width in Pixels", {"n"}},
	{OptionID::TimeRange, "--time-range", "-t", false, "Set Time Range in seconds", {"start-time", "finish-time"}},
	{OptionID::WhiteBackground, "--white-background", "", false, "White Background (instead of black) with inverted heatmap palette", {}},
	{OptionID::WindowFunction, "--window", "-W", false, "Set the window function", {"name"}},
	{OptionID::ShowWindows, "--show-windows", "", false, "Show a list of available window functions", {}},
	{OptionID::SpectrumMode, "--spectrum", "", false, "Plot a Spectrum instead of Spectrogram", {}},

#ifdef FS_AVAILABLE
	{OptionID::Recursive, "--recursive", "-r", false, "Recursive directory traversal", {}},
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

#ifdef FS_AVAILABLE
	bool recursiveDirectoryTraversal{false};
#endif

};

} // namespace Options

#endif // OPTIONS_H
