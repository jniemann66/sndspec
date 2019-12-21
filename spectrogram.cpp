#include "spectrogram.h"
#include "window.h"
#include "reader.h"
#include "spectrum.h"
#include "renderer.h"
#include "raiitimer.h"

#include <iostream>
#include <iomanip>

#include <cassert>

void Sndspec::Spectrogram::makeSpectrogramFromFile(const Sndspec::Parameters &parameters)
{
	if(parameters.getInputFiles().empty()) {
		std::cout << "No input files specified. Nothing to do." << std::endl;
	}

	static const int reservedChannels(2); // stereo (most common use case)

	// prepare a renderer
	Renderer renderer(parameters.getImgWidth(), parameters.getImgHeight());

	auto fftSize = Spectrum::selectBestFFTSizeFromSpectrumSize(renderer.getPlotHeight());
	auto spectrumSize = Spectrum::convertFFTSizeToSpectrumSize(fftSize);
	int plotWidth = renderer.getPlotWidth();

	// make a suitable FFT Window
	Sndspec::Window<double> window;
	window.generate(parameters.getWindowFunction(), fftSize, Sndspec::Window<double>::kaiserBetaFromDecibels(parameters.getDynRange()));

	// prepare storage for spectrogram results
	SpectrogramResults<double> spectrogramData;
	spectrogramData.reserve(reservedChannels);

	// prepare the spectrum analyzers
	std::vector<std::unique_ptr<Spectrum>> analyzers;
	analyzers.reserve(reservedChannels);

	for(const std::string& inputFilename : parameters.getInputFiles()) {
		std::cout << "Opening input file: " << inputFilename << " ... ";
		Sndspec::Reader<double> r(inputFilename, fftSize, plotWidth);

		if(r.getSndFileHandle() == nullptr || r.getSndFileHandle()->error() != SF_ERR_NO_ERROR) {
			std::cout << "couldn't open file !" << std::endl;
		} else {

			SndSpec::RaiiTimer _t;
			std::cout << "ok" << std::endl;
			int nChannels = r.getNChannels();
			std::cout << "channels: " << nChannels << std::endl;

			// provide the reader with the FFT window. The Reader will apply the window to each block it reads.
			r.setWindow(window.getData());

			// resize output storage (according to number of channels)
			spectrogramData.resize(nChannels, std::vector<std::vector<double>>(plotWidth, std::vector<double>(spectrumSize, 0.0)));

			// set specific time range
			if(parameters.hasTimeRange()) {
				r.setStartPos(std::max(0, std::min(static_cast<int>(r.getSamplerate() * parameters.getStart()), r.getNFrames())));
				r.setFinishPos(std::max(0, std::min(static_cast<int>(r.getSamplerate() * parameters.getFinish()), r.getNFrames())));
			}

			for(int ch = 0; ch < nChannels; ch ++) {

				// create a spectrum analyzer for each channel if not already existing
				if(ch + 1 > analyzers.size()) {
					analyzers.emplace_back(new Spectrum(fftSize));
				}

				// give the reader direct write-access to the analyzer input buffer
				r.setChannelBuffer(ch, analyzers.at(ch)->getTdBuf());
			}

			// set a callback function to execute spectrum analysis for each block read
			r.setProcessingFunc([&analyzers, &spectrogramData](int pos, int channel, const double* data) -> void {
				Spectrum* analyzer = analyzers.at(channel).get();
				assert(data == analyzer->getTdBuf());
				analyzer->exec();
				analyzer->getMagSquared(spectrogramData[channel][pos]); // magSquared avoids having do to square root !
			});

			// read (and analyze) the file
			r.readDeinterleaved();

			// scale the data into dB
			renderer.setChannelsEnabled(convertToDb(spectrogramData, /* magSquared = */ true));

			// set render parameters
			double startTime = static_cast<double>(r.getStartPos()) / r.getSamplerate();
			double finishTime = static_cast<double>(r.getFinishPos()) / r.getSamplerate();
			renderer.setNyquist(r.getSamplerate() / 2);
			renderer.setFreqStep(5000);
			renderer.setNumTimeDivs(5);
			renderer.setInputFilename(inputFilename);
			renderer.setStartTime(startTime);
			renderer.setFinishTime(finishTime);
			renderer.setDynRange(parameters.getDynRange());

			std::cout << "Rendering ... ";
			// main plot area
			renderer.renderSpectrogram(parameters, spectrogramData);

			if(parameters.hasWhiteBackground()) {
				renderer.makeNegativeImage();
			}

			std::cout << "Done\n";

			// determine output filename
			std::string outputFilename;
			if(parameters.getOutputPath().empty()) {
				outputFilename = replaceFileExt(inputFilename, "png");
			} else {
				outputFilename = enforceTrailingSeparator(parameters.getOutputPath()) + getFilenameOnly(replaceFileExt(inputFilename, "png"));
			}

			if(!outputFilename.empty()) {
				std::cout << "Saving to " << outputFilename << std::flush;
				if(renderer.writeToFile(outputFilename)) {
					std::cout << " ... OK" << std::endl;
				} else {
					std::cout << " ... ERROR" << std::endl;
				}
			} else {
				std::cout << "Error: couldn't deduce output filename" << std::endl;
			}

			renderer.clear();

		} // ends successful file-open
	} // ends loop over files
}

std::vector<bool> Sndspec::Spectrogram::convertToDb(SpectrogramResults<double> &s, bool fromMagSquared)
{
	int numChannels = s.size();
	int numSpectrums = s.at(0).size();
	int numBins = s.at(0).at(0).size();
	std::vector<bool> hasSignal(numChannels, false);

	for(int c = 0; c < numChannels; c++) {

		// find peak
		double peak{0.0};
		for(int x = 0; x < numSpectrums; x++) {
			for(int b = 0; b < numBins; b++) {
				peak = std::max(peak, s[c][x][b]);
			}
		}

		if(std::fpclassify(peak) != FP_ZERO) {

			hasSignal[c] = true;
			double dBMult = fromMagSquared ? 10.0 : 20.0;

			// set a floor to avoid log(0) problems
			double floor = std::max(std::numeric_limits<double>::min(), peak * pow(10.0, -300.0 / dBMult)); // 300dB below peak or smallest normal number
			assert(std::isnormal(floor));

			// function to convert to dB
			auto scaleFunc = [scale = 1.0 / peak, dBMult, floor] (double v) -> double {
				return dBMult * std::log10(std::max(scale * v, floor));
			};

			// scale the data
			for(int x = 0; x < numSpectrums; x++) {
				std::transform (s[c][x].begin(), s[c][x].end(), s[c][x].begin(), scaleFunc);
			}
		}
	}

	return hasSignal;
}
