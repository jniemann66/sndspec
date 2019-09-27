#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>

namespace Sndspec {

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

	int getIngHeight() const;
	void setIngHeight(int value);

	double getDynRange() const;
	void setDynRange(double value);

	int getFftSize() const;
	void setFftSize(int value);

private:
	std::vector<std::string> inputFiles;
	std::string outputPath;
	int imgWidth{640};
	int ingHeight{480};
	double dynRange{190};
	int fftSize{512};

};

} // namespace Options

#endif // OPTIONS_H
