#ifndef READER_H
#define READER_H

#include <functional>
#include <array>
#include <vector>
#include <string>
#include <memory>

#include <sndfile.hh>

namespace Sndspec {

template <typename T>
class Reader
{
public:
	using ProcessingFunc = std::function<void(int64_t frame, int channel, const std::vector<T>& data)>;
	Reader(const std::string& filename, const ProcessingFunc& f, int blockSize, int w)
	    : filename(filename), processingFunc(f), blockSize(blockSize), w(w)
	{
		sndFileHandle = std::make_unique<SndfileHandle>(Reader::filename);
		if(sndFileHandle.get() != nullptr) {
			startPos = 0LL;
			finishPos = sndFileHandle->frames();
			nChannels = sndFileHandle->channels();
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
		interval = (finishPos - startPos) / w;
	}

	void read()
	{
		std::vector<T> inputBuffer(nChannels * blockSize);
		std::vector<std::vector<T> > channelBuffers;
		for(int ch = 0; ch < nChannels; ch ++) {
			channelBuffers.emplace_back(blockSize, 0.0);
		}

		int64_t startFrame = startPos;
		for(int x = 0; x < w; x++) {

			sndFileHandle->seek(startFrame, SEEK_SET);
			int64_t framesRead = sndFileHandle->readf(inputBuffer.data(), blockSize);

			// deinterleave
			const T* p = inputBuffer.data();
			for(int64_t f = 0; f < framesRead; f++) {
				for(int ch = 0; ch < nChannels; ch++) {
					channelBuffers[f][ch] = *p++;
				}
			}

			// call processing function
			for(int ch = 0; ch < nChannels; ch++) {
				processingFunc(startFrame, ch, inputBuffer.at(ch));
			}

			// advance
			startFrame += interval;
		}
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
};

} // namespace Sndspec

#endif // READER_H
