/*
* Copyright (C) 2019 - 2020 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

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
