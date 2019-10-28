#include "directory.h"

#include <experimental/filesystem>

#include <iostream>

namespace fs = std::experimental::filesystem;

namespace Sndspec {

std::vector<std::string> expand(const std::string &path, const std::vector<std::string> &extensions, bool recursive)
{
	std::vector<std::string> retval;

	if(recursive) {
		for(const auto& item : fs::recursive_directory_iterator(path)) {
			const auto fn = item.path().filename().string();
			if(fs::is_regular_file(item.status())) {
				for(const auto& ext : extensions) {
					if(item.path().extension().string() == ext) {
						retval.push_back(fn);
					}
				}
			}
		}
	} else {
		for(const auto& item : fs::directory_iterator(path)) {
			const auto fn = item.path().filename().string();
			if(fs::is_regular_file(item.status())) {
				retval.push_back(fn);
			}
		}

	}

	return retval;
}


} // namespace Sndspec
