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
	std::vector<std::variant<int, double, std::string>> args;
};

const std::vector<Option> options
{
	{OptionID::DynRange, "--dyn-range", "", 1, false, "Set Dynamic Range in dB", {190}},
	{OptionID::OutputDir, "--output-dir", "-o", 1, false, "Set Output directory", {""}},
	{OptionID::Height, "--height","-h", 1, false, "Set Image Height in Pixels", {480}},
	{OptionID::Width, "--width", "-w", 1, false, "Set Image Width in Pixels", {640}},
	{OptionID::TimeRange, "--time-range", "-t", 2, false, "Set Time Range", {0.0, 0.0}},
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

private:
	std::vector<std::string> inputFiles;
	std::string outputPath;
	int imgWidth{1024};
	int imgHeight{768};
	double dynRange{190};
	double start{0.0};
	double finish{0.0};
};

} // namespace Options

#endif // OPTIONS_H
