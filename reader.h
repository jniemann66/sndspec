#ifndef READER_H
#define READER_H

#include <iostream>
#include <functional>
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <cmath>

#include <sndfile.hh>

namespace Sndspec {

template <typename T>
class Reader
{
public:
	using ProcessingFunc = std::function<void(int pos, int channel, const T* data)>;
	Reader(const std::string& filename, int blockSize, int w)
		: filename(filename), blockSize(blockSize), w(w)
	{
		sndFileHandle = std::make_unique<SndfileHandle>(Reader::filename);
		if(sndFileHandle.get() != nullptr) {
			setStartPos(0LL);
			setFinishPos(sndFileHandle->frames());
			nChannels = sndFileHandle->channels();
			samplerate = sndFileHandle->samplerate();
			nFrames = sndFileHandle->frames();
			channelBuffers.resize(nChannels, nullptr);

			// placeholder function
			processingFunc = [](int pos, int ch, const T* data) -> void {
				(void)data; // unused
				std::cout << "pos " << pos << " ch " << ch << std::endl;
			};
		}
	}

	const SndfileHandle* getSndFileHandle() const
	{
		return sndFileHandle.get();
	}

	int64_t getStartPos() const
	{
		return startPos;
	}
	void setStartPos(const int64_t &value)
	{
		startPos = value;
		interval = (finishPos - startPos) / w;
	}

	int64_t getFinishPos() const
	{
		return finishPos;
	}

	void setFinishPos(const int64_t &value)
	{
		finishPos = value;
		interval = std::ceil((finishPos - startPos) / w);
	}

	void readDeinterleaved()
	{
		if(!window.empty() && window.size() != blockSize) { // incorrect window size
			return;
		}

		std::vector<T> inputBuffer(nChannels * blockSize);

		int64_t startFrame = startPos;
		for(int x = 0; x < w; x++) {

			sndFileHandle->seek(startFrame, SEEK_SET);
			int64_t framesRead = sndFileHandle->readf(inputBuffer.data(), blockSize);

			if (framesRead < blockSize) {
				// pad with trailing zeroes
				for(size_t i = static_cast<size_t>(framesRead); i < inputBuffer.size(); i++) {
					inputBuffer[i] = 0.0;
				}
			}

			// deinterleave
			const T* p = inputBuffer.data();
			if(window.empty()) {
				for(int64_t f = 0; f < framesRead; f++) {
					for(int ch = 0; ch < nChannels; ch++) {
						channelBuffers[ch][f] = *p++;
					}
				}
			} else {
				for(int64_t f = 0; f < framesRead; f++) {
					for(int ch = 0; ch < nChannels; ch++) {
						channelBuffers[ch][f] = *p++ * window[f];
					}
				}
			}

			// call processing function
			for(int ch = 0; ch < nChannels; ch++) {
				processingFunc(x, ch, channelBuffers.at(ch));
			}

			// advance
			startFrame += interval;
		}
	}

	std::vector<T> getWindow() const
	{
		return window;
	}

	void setWindow(const std::vector<T> &value)
	{
		window = value;
	}

	void setChannelBuffer(int channel, T* pBuf)
	{
		channelBuffers[channel] = pBuf;
	}

	int getNChannels() const
	{
		return nChannels;
	}

	void setProcessingFunc(const ProcessingFunc &value)
	{
		processingFunc = value;
	}

	int getSamplerate() const
	{
		return samplerate;
	}

	void setSamplerate(int value)
	{
		samplerate = value;
	}

	int getNFrames() const
	{
		return nFrames;
	}

	void setNFrames(int value)
	{
		nFrames = value;
	}

	int getBlockSize() const
	{
		return blockSize;
	}
	void setBlockSize(int value)
	{
		blockSize = value;
	}

private:
	std::string filename;
	ProcessingFunc processingFunc;
	int blockSize;
	int64_t startPos;
	int64_t finishPos;
	int64_t interval;
	int w;
	std::unique_ptr<SndfileHandle> sndFileHandle;
	int nChannels;
	int samplerate;
	int64_t nFrames;
	std::vector<T> window;
	std::vector<T*> channelBuffers;
};

} // namespace Sndspec

#endif // READER_H
