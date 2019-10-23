#include "parameters.h"

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
				inputFiles.push_back(*argsIt);
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
		case Help:
			++argsIt;
			return showHelp();
		}
	}

	return {};
}

std::string Parameters::showHelp()
{
	std::string helpString{"Usage: sndspec filename [filename2 ...] [options]\n\n"};
	for(const auto& option : options) {
		if(!option.shortOption.empty()) {
			helpString.append(option.shortOption);
			helpString.append(", ");
		}

		helpString.append(option.longOption).append("\t\t").append(option.description).append("\n");
	}

	return helpString;
}

} // namespace Sndspec

