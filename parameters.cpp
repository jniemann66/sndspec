/*
* Copyright (C) 2019 - 2020 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#include "parameters.h"
#include "window.h"

#ifdef FS_AVAILABLE
#include "directory.h"
#endif

#ifdef SNDSPEC_VERSION
#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)
#define VERSION_STRING STRINGIFY(SNDSPEC_VERSION)
#endif

#include <iostream>
#include <cmath>
#include <sstream>
#include <iterator>
#include <regex>

namespace Sndspec {

constexpr int minImgWidth = 160;
constexpr int minImgHeight = 160;

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
			if((*argsIt == option.longOption) || (!option.shortOption.empty() && (*argsIt == option.shortOption))) {
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
			}
			break;
		case OutputDir:
			if(++argsIt != args.cend()) {
				outputPath = *argsIt;
				++argsIt;	
			}
			break;
		case Height:
			if(++argsIt != args.cend()) {
                imgHeight = std::max(minImgHeight, std::stoi(*argsIt));
				++argsIt;	
			}
			break;
		case Width:
			if(++argsIt != args.cend()) {
                imgWidth = std::max(minImgWidth, std::stoi(*argsIt));
				++argsIt;	
			}
			break;
		case TimeRange:
			if(++argsIt != args.cend()) {
				timeRange = true;
				start = std::max(0.0, std::stod(*argsIt));
				if(++argsIt != args.cend()) {
					finish = std::max(0.0, std::stod(*argsIt));
				}
				++argsIt;	
			}
			break;
		case WhiteBackground:
			whiteBackground = true;
			++argsIt;
			break;
		case WindowFunction:
			if(++argsIt != args.cend()) {
				showWindowFunctionLabel = true;
				auto wp = Window<double>::findWindow(*argsIt);
				if(wp.windowType == Unknown) {
					windowFunction = "kaiser";
					windowFunctionDisplayName = "Kaiser";
				} else {
					windowFunction = wp.name;
					windowFunctionDisplayName = wp.displayName;
				}
				++argsIt;
			}
			break;
		case ShowWindows:
			showWindows = true;
			++argsIt;
			return showWindowList();
		case SpectrumMode:
			spectrumMode = true;
			++argsIt;
			break;
		case Smoothing:
			if(++argsIt != args.cend()) {
				std::string s{*argsIt};

				// convert name to lowercase
				std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) -> unsigned char {
					return std::tolower(c);
				});

				if(s.compare(0, 6, "moving") == 0) {
					spectrumSmoothingMode = MovingAverage;
				} else if(s.compare(0, 4, "peak") == 0) {
					spectrumSmoothingMode = Peak;
				} else if(s.compare(0, 4, "none") == 0 ) {
					spectrumSmoothingMode = None;
				}
				++argsIt;
				break;
			}
			break;
		case Channel:
		{
			std::vector<std::string> channelArgs;
			while (++argsIt != args.cend() && argsIt->compare(0, 1, "-") != 0) {
				channelArgs.push_back(*argsIt);
			}
			processChannelArgs(channelArgs);
			break;
		}
		case LinearMag:
			linearMag = true;
			++argsIt;
			break;

#ifdef FS_AVAILABLE
		case Recursive:
			recursiveDirectoryTraversal = true;
			++argsIt;
			break;
#endif

#ifdef SNDSPEC_VERSION
		case Version:
			++argsIt;
			return std::string{VERSION_STRING};
#endif
		case Help:
			++argsIt;
			return showHelp();
		}
	}

#ifdef FS_AVAILABLE
	std::vector<std::string> expandedFileList;
	for(const auto& path : inputFiles) {
		auto list = expand(path, fileTypes, recursiveDirectoryTraversal);
		expandedFileList.insert(expandedFileList.end(), list.begin(), list.end());
	}
	inputFiles = expandedFileList;
#endif

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

std::string Parameters::showWindowList()
{
	const auto names = Sndspec::getWindowNames();
	const char* const delim = "\n";

	std::ostringstream oss;
	std::copy(names.begin(), names.end(), std::ostream_iterator<std::string>(oss, delim));
	return oss.str();
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

std::string Parameters::getWindowFunction() const
{
	return windowFunction;
}

bool Parameters::getShowWindows() const
{
	return showWindows;
}

std::string Parameters::getWindowFunctionDisplayName() const
{
	return windowFunctionDisplayName;
}

void Parameters::setWindowFunctionDisplayName(const std::string &value)
{
	windowFunctionDisplayName = value;
}

bool Parameters::getShowWindowFunctionLabel() const
{
	return showWindowFunctionLabel;
}

bool Parameters::getSpectrumMode() const
{
	return spectrumMode;
}

SpectrumSmoothingMode Parameters::getSpectrumSmoothingMode() const
{
	return spectrumSmoothingMode;
}

std::set<int> Parameters::getSelectedChannels() const
{
	return selectedChannels;
}

ChannelMode Parameters::getChannelMode() const
{
	return channelMode;
}

bool Parameters::getLinearMag() const
{
	return linearMag;
}

void Parameters::setLinearMag(bool value)
{
	linearMag = value;
}

void Parameters::setChannelMode(const ChannelMode &value)
{
	channelMode = value;
}

void Parameters::setSelectedChannels(const std::set<int> &value)
{
	selectedChannels = value;
}

void Parameters::setSpectrumSmoothingMode(const SpectrumSmoothingMode &value)
{
	spectrumSmoothingMode = value;
}

void Parameters::setSpectrumMode(bool value)
{
	spectrumMode = value;
}

void Parameters::setShowWindowFunctionLabel(bool value)
{
	showWindowFunctionLabel = value;
}

void Parameters::setShowWindows(bool value)
{
	showWindows = value;
}

void Parameters::setWindowFunction(const std::string &value)
{
	windowFunction = value;
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

void Parameters::processChannelArgs(const std::vector<std::string>& args)
{
	selectedChannels.clear();
	channelMode = Normal;

	std::string s;
	for(const auto& arg : args) {
		s.append(arg);
	}

	// convert s to lowercase
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) -> unsigned char {
		return std::tolower(c);
	});

	std::regex rx{"(\\d{1}|[lr]|all|sum|diff|norm)", std::regex::icase};
	auto it_begin = std::sregex_iterator(s.begin(), s.end(), rx);
	auto it_end = std::sregex_iterator();

	for(std::regex_iterator it = it_begin; it != it_end; ++it){
		std::smatch match = *it;
		std::string match_str = match.str();
		if(match_str == "l") {
			selectedChannels.insert(0);
		} else if(match_str == "r") {
			selectedChannels.insert(1);
		} else if(match_str == "all") {
			selectedChannels.clear();
		} else if(match_str == "sum") {
			channelMode = Sum;
		} else if(match_str == "diff") {
			channelMode = Difference;
		} else if(match_str == "norm") {
			channelMode = Normal;
		} else {
			try {
				selectedChannels.insert(std::stoi(match_str));
			}
			catch (...) {}
		}
	}
}

} // namespace Sndspec
