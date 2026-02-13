/*
* Copyright (C) 2019 - 2026 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#include "parameters.h"
#include "window.h"
#include "directory.h"

#ifdef SNDSPEC_VERSION
#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)
#define VERSION_STRING STRINGIFY(SNDSPEC_VERSION)
#endif

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

void Parameters::setInputFiles(const std::vector<std::string> &val)
{
	inputFiles = val;
}

std::string Parameters::getOutputPath() const
{
	return outputPath;
}

void Parameters::setOutputPath(const std::string &val)
{
	outputPath = val;
}

int Parameters::getImgWidth() const
{
	return imgWidth;
}

void Parameters::setImgWidth(int val)
{
	imgWidth = val;
}

int Parameters::getImgHeight() const
{
	return imgHeight;
}

void Parameters::setIngHeight(int val)
{
	imgHeight = val;
}

double Parameters::getDynRange() const
{
	return dynRange;
}

void Parameters::setDynRange(double val)
{
	dynRange = val;
}

std::string Parameters::fromArgs(const std::vector<std::string> &args)
{
	auto argsIt = args.cbegin();
	while (argsIt != args.cend()) {
		OptionID optionID = OptionID::Filenames; // unrecognized options to be treated as filenames

		// option search
		for (const auto& option : options) {
			if ((*argsIt == option.longOption) || (!option.shortOption.empty() && (*argsIt == option.shortOption))) {
				optionID = option.optionID;
				break;
			}
		}

		// process option
		switch (optionID) {
		case Filenames:
			// keep reading filenames until end reached, or another option detected
			do {
				inputFiles.push_back(*argsIt);
				argsIt++;
			} while (argsIt != args.cend() && argsIt->compare(0, 1, "-") != 0);
			break;

		case DynRange:
			if (++argsIt != args.cend()) {
				dynRange = std::abs(std::stod(*argsIt));
				++argsIt;
			}
			break;

		case OutputDir:
			if (++argsIt != args.cend()) {
				outputPath = *argsIt;
				++argsIt;
			}
			break;

		case Height:
			if (++argsIt != args.cend()) {
				imgHeight = std::max(minImgHeight, std::stoi(*argsIt));
				++argsIt;
			}
			break;

		case Width:
			if (++argsIt != args.cend()) {
				imgWidth = std::max(minImgWidth, std::stoi(*argsIt));
				++argsIt;
			}
			break;

		case TimeRange:
			if (++argsIt != args.cend()) {
				timeRange = true;
				start = std::max(0.0, std::stod(*argsIt));
				if (++argsIt != args.cend()) {
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
			if (++argsIt != args.cend()) {
				showWindowFunctionLabel = true;
				auto wp = Window<double>::findWindow(*argsIt);
				if (wp.windowType == Unknown) {
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
			if (++argsIt != args.cend()) {
				std::string s{*argsIt};

				// convert name to lowercase
				std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) -> unsigned char {
					return std::tolower(c);
				});

				if (s.compare(0, 6, "moving") == 0) {
					spectrumSmoothingMode = MovingAverage;
				} else if (s.compare(0, 4, "peak") == 0) {
					spectrumSmoothingMode = Peak;
				} else if (s.compare(0, 4, "none") == 0 ) {
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
			dynRange = 100.0;
			++argsIt;
			break;

		case FrequencyStep:
			if (++argsIt != args.cend()) {
				frequencyStep = std::max(1, std::stoi(*argsIt));
				++argsIt;
			}
			break;

		case PeakSelection:
			if (++argsIt != args.cend()) {
				topN = std::max(1, std::stoi(*argsIt));
				++argsIt;
				if (argsIt != args.cend() && !argsIt->empty() && argsIt->at(0) != '-') {
					std::cout << *argsIt << std::endl;
					try {
						// look for potentially another number (min-spacing in Hz)
						const double d = std::stod(*argsIt);
						topN_minSpacing = std::max(1.0, d);
						std::cout << "minimum spacing between peaks set to " << d << std::endl;
						++argsIt;
					} catch (const std::invalid_argument& e) {
					} catch (const std::out_of_range& e) {
					}
				}
			}
			break;

		case PlotWindowFunction:
			if (++argsIt != args.cend()) {
				plotWindowFunction = true;
				showWindowFunctionLabel = true;
				spectrumMode = true;
				auto wp = Window<double>::findWindow(*argsIt);
				if (wp.windowType == Unknown) {
					windowFunction = "kaiser";
					windowFunctionDisplayName = "Kaiser";
				} else {
					windowFunction = wp.name;
					windowFunctionDisplayName = wp.displayName;
				}
				++argsIt;

				while (argsIt != args.cend() && !argsIt->empty() && argsIt->at(0) != '-') {
					std::string s{*argsIt};

					// convert name to lowercase
					std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) -> unsigned char {
						return std::tolower(c);
					});

					if (s.compare(0, 4, "time") == 0) {
						linearMag = true;
						dynRange = 100.0;
						plotTimeDomain = true;
						++argsIt;
					} else {
						try {
							// look for a number (parameter for window function)
							const double d = std::stod(s);
							windowFunctionParameters.push_back(std::max(0.0, d));
							std::cout << "applying additional window function parameter: " << d << std::endl;
							++argsIt;
						} catch (const std::invalid_argument& e) {
						} catch (const std::out_of_range& e) {
						}
					}
				} // ends while () loop
			}
			break;

		case Recursive:
			recursiveDirectoryTraversal = true;
			++argsIt;
			break;

#ifdef SNDSPEC_VERSION
		case Version:
			++argsIt;
			return std::string{VERSION_STRING};
#endif

		case Zoom:
			if (++argsIt != args.cend()) {
				// if there is a number, interpret it as zoom factor
				try {
					const double d = std::stod(*argsIt);
					horizZoomFactor = std::max(1.0, d);
					std::cout << "frequency zoom-factor set to " << horizZoomFactor << std::endl;
				} catch (const std::invalid_argument& e) {
				} catch (const std::out_of_range& e) {
				}
			}
			break;

		case Help:
			++argsIt;
			return showHelp();

		} // ends switch(...)
	} // ends while(...)

	std::vector<std::string> expandedFileList;
	for (const auto& path : inputFiles) {
		auto list = expand(path, fileTypes, recursiveDirectoryTraversal);
		expandedFileList.insert(expandedFileList.end(), list.begin(), list.end());
	}
	inputFiles = expandedFileList;

	return {};
}

std::string Parameters::showHelp()
{
	static const int col0width = 50;
	std::string helpString{"Usage: sndspec filename [filename2 ...] [options]\n\n"};
	for (const auto& option : options) {
		std::string line;
		if (!option.shortOption.empty()) {
			line.append(option.shortOption);
			line.append(", ");
		}

		line.append(option.longOption).append(" ");
		for (const auto& arg : option.args) {
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

void Parameters::setHasTimeRange(bool val)
{
	timeRange = val;
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

void Parameters::setWindowFunctionDisplayName(const std::string &val)
{
	windowFunctionDisplayName = val;
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

int Parameters::getFrequencyStep() const
{
	return frequencyStep;
}

std::optional<int> Parameters::getTopN() const
{
	return topN;
}

std::optional<double> Parameters::getTopN_minSpacing() const
{
	return topN_minSpacing;
}

bool Parameters::getPlotWindowFunction() const
{
	return plotWindowFunction;
}

void Parameters::setPlotWindowFunction(bool val)
{
	plotWindowFunction = val;
}

bool Parameters::getPlotTimeDomain() const
{
	return plotTimeDomain;
}

double Parameters::getHorizZoomFactor() const
{
	return horizZoomFactor;
}

std::vector<double> Parameters::getWindowFunctionParameters() const
{
	return windowFunctionParameters;
}

void Parameters::setWindowFunctionParameters(const std::vector<double>& newWindowFunctionParameters)
{
	windowFunctionParameters = newWindowFunctionParameters;
}

void Parameters::setHorizZoomFactor(double newHorizZoomFactor)
{
	horizZoomFactor = newHorizZoomFactor;
}

void Parameters::setPlotTimeDomain(bool val)
{
	plotTimeDomain = val;
}

void Parameters::setTopN_minSpacing(std::optional<double> val)
{
	topN_minSpacing = val;
}

void Parameters::setTopN(std::optional<int> val)
{
	topN = val;
}

void Parameters::setFrequencyStep(int val)
{
	frequencyStep = val;
}

void Parameters::setLinearMag(bool val)
{
	linearMag = val;
}

void Parameters::setChannelMode(const ChannelMode &val)
{
	channelMode = val;
}

void Parameters::setSelectedChannels(const std::set<int> &val)
{
	selectedChannels = val;
}

void Parameters::setSpectrumSmoothingMode(const SpectrumSmoothingMode &val)
{
	spectrumSmoothingMode = val;
}

void Parameters::setSpectrumMode(bool val)
{
	spectrumMode = val;
}

void Parameters::setShowWindowFunctionLabel(bool val)
{
	showWindowFunctionLabel = val;
}

void Parameters::setShowWindows(bool val)
{
	showWindows = val;
}

void Parameters::setWindowFunction(const std::string &val)
{
	windowFunction = val;
}

void Parameters::setHasWhiteBackground(bool val)
{
	whiteBackground = val;
}

void Parameters::setStart(double val)
{
	start = val;
}

void Parameters::setFinish(double val)
{
	finish = val;
}

void Parameters::processChannelArgs(const std::vector<std::string>& args)
{
	selectedChannels.clear();
	channelMode = Normal;

	std::string s;
	for (const auto& arg : args) {
		s.append(arg);
	}

	// convert s to lowercase
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) -> unsigned char {
		return std::tolower(c);
	});

	std::regex rx{"(\\d{1}|[lr]|all|sum|diff|norm)", std::regex::icase};
	auto it_begin = std::sregex_iterator(s.begin(), s.end(), rx);
	auto it_end = std::sregex_iterator();

	for (std::regex_iterator it = it_begin; it != it_end; ++it){
		std::smatch match = *it;
		std::string match_str = match.str();
		if (match_str == "l") {
			selectedChannels.insert(0);
		} else if (match_str == "r") {
			selectedChannels.insert(1);
		} else if (match_str == "all") {
			selectedChannels.clear();
		} else if (match_str == "sum") {
			channelMode = Sum;
		} else if (match_str == "diff") {
			channelMode = Difference;
		} else if (match_str == "norm") {
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
