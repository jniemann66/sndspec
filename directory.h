#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <vector>

namespace Sndspec {

constexpr bool enableDirectoryTraversal = false;

std::vector<std::string> expand(const std::string& path, const std::vector<std::string>& extensions, bool recursive = false);

} // namespace Sndspec


#endif // DIRECTORY_H
