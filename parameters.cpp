#include "parameters.h"

#include "directory.h"

namespace Sndspec {

std::vector<std::string> Parameters::getInputFiles() const
{
	return inputFiles;
}

void Parameters::setInputFiles(const std::vector<std::string> &value)
{
	inputFiles = value;
}

std::string Parameters::getOutputPath() const
{
	return outputPath;
}

void Parameters::setOutputPath(const std::string &value)
{
	outputPath = value;
}

int Parameters::getImgWidth() const
{
	return imgWidth;
}

void Parameters::setImgWidth(int value)
{
	imgWidth = value;
}

int Parameters::getImgHeight() const
{
	return imgHeight;
}

void Parameters::setIngHeight(int value)
{
	imgHeight = value;
}

double Parameters::getDynRange() const
{
	return dynRange;
}

void Parameters::setDynRange(double value)
{
	dynRange = value;
}

std::string Parameters::fromArgs(const std::vector<std::string> &args)
{
	auto argsIt = args.cbegin();
	while(argsIt != args.cend()) {
		OptionID optionID = OptionID::Filenames; // unrecognized options to be treated as filenames

		// option search
		for(const auto& option : options) {
			if((argsIt->compare(option.longOption) == 0) || (!option.shortOption.empty() && (argsIt->compare(option.shortOption) == 0))) {
				optionID = option.optionID;
				break;
			}
		}

		// process option
		switch(optionID) {
		case Filenames:
			// keep reading filenames until end reached, or another option detected
			do {
				if(enableDirectoryTraversal) {
					inputFiles.insert(inputFiles.begin(), inputFiles.end(), expand(*argsIt, {".wav"}).end());
				} else {
					inputFiles.push_back(*argsIt);
				}
				argsIt++;
			} while (argsIt != args.cend() && argsIt->compare(0, 1, "-") != 0);
			break;
		case DynRange:
			if(++argsIt != args.cend()) {
				dynRange = std::abs(std::stod(*argsIt));
				++argsIt;
				break;
			}
		case OutputDir:
			if(++argsIt != args.cend()) {
				outputPath = *argsIt;
				++argsIt;
				break;
			}
		case Height:
			if(++argsIt != args.cend()) {
				imgHeight = std::max(160, std::stoi(*argsIt));
				++argsIt;
				break;
			}
		case Width:
			if(++argsIt != args.cend()) {
				imgWidth = std::max(160, std::stoi(*argsIt));
				++argsIt;
				break;
			}
		case TimeRange:
			if(++argsIt != args.cend()) {
				timeRange = true;
				start = std::max(0.0, std::stod(*argsIt));
				if(++argsIt != args.cend()) {
					finish = std::max(0.0, std::stod(*argsIt));
				}
				++argsIt;
				break;
			}
		case WhiteBackground:
			whiteBackground = true;
			++argsIt;
			break;
		case Help:
			++argsIt;
			return showHelp();
		}
	}

	return {};
}

std::string Parameters::showHelp()
{
	static const int col0width = 50;
	std::string helpString{"Usage: sndspec filename [filename2 ...] [options]\n\n"};
	for(const auto& option : options) {
		std::string line;
		if(!option.shortOption.empty()) {
			line.append(option.shortOption);
			line.append(", ");
		}

		line.append(option.longOption).append(" ");
		for(const auto& arg : option.args) {
			line.append("<").append(arg).append("> ");
		}
		line.append(std::max(0, col0width - static_cast<int>(line.length())), ' ');
		line.append(option.description).append("\n");
		helpString.append(line);
	}

	return helpString;
}

double Parameters::getStart() const
{
	return start;
}

double Parameters::getFinish() const
{
	return finish;
}

bool Parameters::hasTimeRange() const
{
	return timeRange;
}

void Parameters::setHasTimeRange(bool value)
{
	timeRange = value;
}

bool Parameters::hasWhiteBackground() const
{
	return whiteBackground;
}

void Parameters::setHasWhiteBackground(bool value)
{
	whiteBackground = value;
}

void Parameters::setStart(double value)
{
	start = value;
}

void Parameters::setFinish(double value)
{
	finish = value;
}

} // namespace Sndspec

