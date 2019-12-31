/*
* Copyright (C) 2019 - 2020 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "fsdetect.h"

#include <string>
#include <algorithm>
#include <vector>

namespace Sndspec {

template <typename DirIteratorType>
std::vector<std::string> expand(const std::string &path, const std::vector<std::string> &extensions)
{
	std::vector<std::string> retval;

	if(fs::is_regular_file(fs::status(path))) {
		retval.push_back(path);
	} else if (fs::is_directory(fs::status(path))) {
		for(const auto& item : DirIteratorType(path)) {
			const auto fn = item.path().string();
			if(fs::is_regular_file(item.status())) {
				for(auto ext : extensions) {
					std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
						return std::tolower(c);
					});
					if(item.path().extension().string().compare(ext) == 0) {
						retval.push_back(fn);
					}
				}
			}
		}
	}

	return retval;
}

std::vector<std::string> expand(const std::string& path, const std::vector<std::string>& extensions, bool recursive = false)
{
	if(recursive) {
		return expand<fs::recursive_directory_iterator>(path, extensions);
	} else {
		return expand<fs::directory_iterator>(path, extensions);
	}
};

} // namespace Sndspec

#endif // DIRECTORY_H
