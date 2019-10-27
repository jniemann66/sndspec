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
	{OptionID::Help, "--help", "", false, "Help", {}}
};

class Parameters
{
public:
	Parameters() = default;
	std::string fromArgs(const std::vector<std::string>& args);
	std::string showHelp();

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
