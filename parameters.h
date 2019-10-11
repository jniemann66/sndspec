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

	int getImgHeight() const;
	void setIngHeight(int value);

	double getDynRange() const;
	void setDynRange(double value);

private:
	std::vector<std::string> inputFiles;
	std::string outputPath;
	int imgWidth{1024};
	int ingHeight{768};
	double dynRange{190};
};

} // namespace Options

#endif // OPTIONS_H
