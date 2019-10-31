#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <vector>

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif

namespace Sndspec {

#if defined(__cpp_lib_experimental_filesystem) || defined(__cpp_lib_filesystem)

const bool directoryTraversalAvailable = true;

template <typename DirIteratorType>
std::vector<std::string> expand(const std::string &path, const std::vector<std::string> &extensions)
{
	std::vector<std::string> retval;

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

#else
const bool directoryTraversalAvailable = false;
#endif // defined(__cpp_lib_experimental_filesystem) || defined(__cpp_lib_filesystem)

} // namespace Sndspec

#endif // DIRECTORY_H
