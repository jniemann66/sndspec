#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>
#include <variant>

namespace Sndspec {

enum OptionID
{
	Filenames,
	DynRange,
	OutputDir,
	Height,
	Width,
	TimeRange,
	WhiteBackground,
	Help
};

struct Option
{
	OptionID optionID;
	std::string longOption;
	std::string shortOption;
	int expectedArgs;
	bool isMandatory;
	std::string description;
	std::vector<std::string> args;
};

const std::vector<Option> options
{
	{OptionID::DynRange, "--dyn-range", "", 1, false, "Set Dynamic Range in dB", {"n"}},
	{OptionID::OutputDir, "--output-dir", "-o", 1, false, "Set Output directory", {"path"}},
	{OptionID::Height, "--height","-h", 1, false, "Set Image Height in Pixels", {"n"}},
	{OptionID::Width, "--width", "-w", 1, false, "Set Image Width in Pixels", {"n"}},
	{OptionID::TimeRange, "--time-range", "-t", 2, false, "Set Time Range in seconds", {"start-time", "finish-time"}},
	{OptionID::WhiteBackground, "--white-background", "", 0, false, "White Background (instead of black) with inverted heatmap palette", {}},
	{OptionID::Help, "--help", "", 0, false, "Help", {}}
};

class Parameters
{
public:
	Parameters() = default;

	// getters and setters
	std::vector<std::string> getInputFiles() const;
	void setInputFiles(const std::vector<std::string> &value);

	std::string getOutputPath() const;
	void setOutputPath(const std::string &value);

	int getImgWidth() const;
	void setImgWidth(int value);

	int getImgHeight() const;
	void setIngHeight(int value);

	double getDynRange() const;
	void setDynRange(double value);

	std::string fromArgs(const std::vector<std::string>& args);

	std::string showHelp();

	double getStart() const;
	double getFinish() const;

	bool getTimeRange() const;
	void setTimeRange(bool value);

	bool getWhiteBackground() const;
	void setWhiteBackground(bool value);

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
};

} // namespace Options

#endif // OPTIONS_H
