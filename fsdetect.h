#ifndef FSDETECT_H
#define FSDETECT_H

#if __has_include(<filesystem>)
        #include <filesystem>
        namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
        #include <experimental/filesystem>
        namespace fs = std::experimental::filesystem;
#endif

#if defined(__cpp_lib_experimental_filesystem) || defined(__cpp_lib_filesystem)
#define FS_AVAILABLE
#endif

#endif // FSDETECT_H
