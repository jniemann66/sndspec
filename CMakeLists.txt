project(sndspec C CXX)
cmake_minimum_required(VERSION 3.6)
find_package(Threads REQUIRED)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_CONFIGURATION_TYPES)

    # single-config build (build type decided by CMAKE_BUILD_TYPE )

    # for single-config builds, add additional configurations
    #set (CMAKE_CXX_FLAGS_RELEASEAVX ${CMAKE_CXX_FLAGS_RELEASE_INIT} " -DUSE_AVX -mavx")
    set (CMAKE_CXX_FLAGS_RELEASEQUADMATH ${CMAKE_CXX_FLAGS_RELEASE_INIT} " -DUSE_QUADMATH")

    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "build type not specified - using default")
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type" FORCE)
    endif()

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "ReleaseQuadmath")
    message(STATUS "build type: '${CMAKE_BUILD_TYPE}'")

endif()

set(SOURCE_FILES
	spectrum.cpp
	spectrogram.cpp
	parameters.cpp
	tests.cpp
        renderer.cpp
  )

if (WIN32)
    # sorry, 64-bit only at the moment ...
    find_library(SNDFILE_LIBRARY NAMES libsndfile-1 libsndfile HINTS ${PROJECT_SOURCE_DIR}/libsndfile/lib)
    #find_library(FFTW_LIBRARY NAMES fftw3-3 PATH ${CMAKE_CURRENT_SOURCE_DIR}/fftw64 ) #todo: why is find_library() looking for this in places other than where I asked it to look ?
    set(FFTW_LIBRARY ${PROJECT_SOURCE_DIR}/fftw64/libfftw3-3.lib)
    find_library(CAIRO_LIBRARY NAMES cairo HINTS ${PROJECT_SOURCE_DIR}/cairo/lib/x64)
    include_directories(libsndfile/include fftw64 cairo/include)
else()
    #Not Windows ...
    if(APPLE)
        # since Mojave, cmake no longer finds these automatically:
        # (see https://stackoverflow.com/questions/54068035/linking-not-working-in-homebrews-cmake-since-mojave)
        include_directories(/usr/local/include)
        link_directories(/usr/local/lib)
    endif()

    add_library(sndspecLib SHARED ${SOURCE_FILES})
    if(CMAKE_BUILD_TYPE STREQUAL "ReleaseQuadmath")
        message(STATUS "Linking with Quadmath library")
        set(CMAKE_CXX_EXTENSIONS TRUE)  # -std=gnu++11 instead of -std=c++11
        target_link_libraries(sndspecLib sndfile fftw3 quadmath ${CMAKE_THREAD_LIBS_INIT})
    else()
        target_link_libraries(sndspecLib sndfile fftw3 ${CMAKE_THREAD_LIBS_INIT})
    endif()

endif()

# common
message(STATUS "libsndfile library location: " ${SNDFILE_LIBRARY})
message(STATUS "fftw library location: " ${FFTW_LIBRARY})
message(STATUS "cairo library location: "  ${CAIRO_LIBRARY})
add_library(sndspecLib ${SOURCE_FILES})
add_executable(sndspec main.cpp)
target_link_libraries(sndspec sndspecLib ${SNDFILE_LIBRARY} ${FFTW_LIBRARY} ${CAIRO_LIBRARY})
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
