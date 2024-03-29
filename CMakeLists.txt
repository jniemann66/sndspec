project(sndspec C CXX)
set(sndspec_VERSION 0.9.04)
cmake_minimum_required(VERSION 3.6)
find_package(Threads REQUIRED)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(SNDSPEC_VERSION=${sndspec_VERSION})

if(NOT CMAKE_CONFIGURATION_TYPES)
    # single-config build (build type decided by CMAKE_BUILD_TYPE )
    # for single-config builds, add additional configurations
    #set (CMAKE_CXX_FLAGS_RELEASEAVX ${CMAKE_CXX_FLAGS_RELEASE_INIT} " -DUSE_AVX -mavx")
    set (CMAKE_CXX_FLAGS_RELEASEQUADMATH ${CMAKE_CXX_FLAGS_RELEASE_INIT} " -DUSE_QUADMATH")
  #  set (CMAKE_CXX_FLAGS_RELEASECUDA ${CMAKE_CXX_FLAGS_RELEASE_INIT} " -DCUFFTW")

    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "build type not specified - using default")
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type" FORCE)
    endif()

  #  set(CMAKE_BUILD_TYPE "ReleaseCUDA" CACHE STRING "Choose the build type" FORCE)

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "ReleaseQuadmath" "ReleaseCUDA")
    message(STATUS "build type: '${CMAKE_BUILD_TYPE}'")
endif()

set(SOURCE_FILES
    spectrum.h
    spectrogram.h
    parameters.h
    tests.h
    renderer.h
    spectrum.cpp
    spectrogram.cpp
    parameters.cpp
    tests.cpp
    renderer.cpp
  )

if (WIN32)
    # sorry, 64-bit only at the moment ...
    find_library(SNDFILE_LIBRARY NAMES libsndfile-1 libsndfile HINTS ${PROJECT_SOURCE_DIR}/libsndfile/lib)

    if(CMAKE_BUILD_TYPE STREQUAL "ReleaseCUDA")
        # put paths to NVIDIA GPU Computing toolkit in here ...
        include_directories(libsndfile/include fftw64 cairo/include "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.1/include")
        set(FFTW_LIBRARY "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.1/lib/x64/cufftw.lib")
    else()
        #find_library(FFTW_LIBRARY NAMES fftw3-3 PATH ${CMAKE_CURRENT_SOURCE_DIR}/fftw64 ) #todo: why is find_library() looking for this in places other than where I asked it to look ?
        include_directories(libsndfile/include fftw64 cairo/include)
        set(FFTW_LIBRARY ${PROJECT_SOURCE_DIR}/fftw64/libfftw3-3.lib)
    endif()

    find_library(CAIRO_LIBRARY NAMES cairo HINTS ${PROJECT_SOURCE_DIR}/cairo/lib/x64)
    add_library(sndspecLib ${SOURCE_FILES})

else()
    #Not Windows ...
    if(APPLE)
        # since Mojave, cmake no longer finds these automatically:
        # (see https://stackoverflow.com/questions/54068035/linking-not-working-in-homebrews-cmake-since-mojave)
        link_directories(/usr/local/lib)
        include_directories(/usr/local/include /usr/local/include/cairo)
    else()
        include_directories(/usr/local/include /usr/include /usr/include/cairo)
    endif()

    # find libsndfile
    find_library(SNDFILE_LIBRARY NAMES sndfile)

    # find fftw or cufftw
    if(CMAKE_BUILD_TYPE STREQUAL "ReleaseCUDA")
        find_library(FFTW_LIBRARY NAMES cufftw)
    else()
        # on my system, fftw library gets put in /usr/lib/x86_64-linux-gnu/ but ymmv. on other systems it may be lib64 etc
        find_library(FFTW_LIBRARY NAMES fftw3 HINTS /usr/lib/x86_64-linux-gnu/)
    endif()

    # find cairo
    find_library(CAIRO_LIBRARY NAMES cairo)
    add_library(sndspecLib SHARED ${SOURCE_FILES})

endif()

# common
message(STATUS "libsndfile library location: " ${SNDFILE_LIBRARY})
message(STATUS "fftw library location: " ${FFTW_LIBRARY})
message(STATUS "cairo library location: "  ${CAIRO_LIBRARY})
add_executable(sndspec main.cpp)

# link all the relevant libraries
if(APPLE)
    target_link_libraries(sndspec sndspecLib ${SNDFILE_LIBRARY} ${FFTW_LIBRARY} ${CAIRO_LIBRARY} )
else()
    if(CMAKE_BUILD_TYPE STREQUAL "ReleaseQuadmath")
        message(STATUS "Linking with Quadmath library")
        set(CMAKE_CXX_EXTENSIONS TRUE)  # -std=gnu++11 instead of -std=c++11
        target_link_libraries(sndspec sndspecLib ${SNDFILE_LIBRARY} ${FFTW_LIBRARY} ${CAIRO_LIBRARY} stdc++fs quadmath)
    else()
        target_link_libraries(sndspec sndspecLib ${SNDFILE_LIBRARY} ${FFTW_LIBRARY} ${CAIRO_LIBRARY} stdc++fs)
    endif()
endif()

target_compile_features(sndspecLib PRIVATE cxx_std_17)
# ---

if(WIN32) # deploy dlls to target directory
add_custom_command(TARGET sndspec POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/libsndfile/bin/libsndfile-1.dll"
        "$<TARGET_FILE_DIR:sndspec>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/cairo/lib/x64/cairo.dll"
        "$<TARGET_FILE_DIR:sndspec>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/fftw64/libfftw3-3.dll"
        "$<TARGET_FILE_DIR:sndspec>"
    )
endif()
