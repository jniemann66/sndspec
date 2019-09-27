project(sndspec C CXX)
cmake_minimum_required(VERSION 3.6)
find_package(Threads REQUIRED)
set(CMAKE_CXX_STANDARD 11)


if(NOT CMAKE_CONFIGURATION_TYPES)

    # single-config build (build type decided by CMAKE_BUILD_TYPE )

    # for single-config builds, add additional configurations
    set (CMAKE_CXX_FLAGS_RELEASEAVX ${CMAKE_CXX_FLAGS_RELEASE_INIT} " -DUSE_AVX -mavx")
    set (CMAKE_CXX_FLAGS_RELEASEQUADMATH ${CMAKE_CXX_FLAGS_RELEASE_INIT} " -DUSE_QUADMATH")

    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "build type not specified - using default")
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type" FORCE)
    endif()

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "ReleaseAVX" "ReleaseQuadmath")

    message(STATUS "build type: '${CMAKE_BUILD_TYPE}'")

endif()

set(SOURCE_FILES
    spectrogram.cpp
	parameters.cpp
  )

if (WIN32)

    find_library(SNDFILE_LIBRARY NAMES libsndfile-1 libsndfile HINTS ${PROJECT_SOURCE_DIR}/libsndfile/lib)
    message(STATUS "libsndfile library location: " ${SNDFILE_LIBRARY})

    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(STATUS "Windows 32-bit")
        include_directories(libsndfile/include fftw32)
        link_directories(libsndfile32/lib fftw32)
    else()
        message(STATUS "Windows 64-bit")
        include_directories(libsndfile/include fftw64)
        link_directories(libsndfile/lib fftw64)
    endif()

    add_library(sndspecLib ${SOURCE_FILES})

    if(MSVC)
        message(STATUS "Visual Studio")
        target_link_libraries(sndspecLib libsndfile-1.lib libfftw3-3.lib ${CMAKE_THREAD_LIBS_INIT})

        if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
            # create additional 64-bit config: ReleaseAVX
            set (CMAKE_CONFIGURATION_TYPES "Debug;Release;ReleaseAVX" CACHE STRING "" FORCE)

            # add /arch:AVX and /DUSE_AVX flags for ReleaseAVX configuration
            set (CMAKE_CXX_FLAGS_RELEASEAVX ${CMAKE_CXX_FLAGS_RELEASE_INIT} " /arch:AVX /DUSE_AVX")

            # same linker flags
            set (CMAKE_EXE_LINKER_FLAGS_RELEASEAVX ${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT})
            set (CMAKE_SHARED_LINKER_FLAGS_RELEASEAVX ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})

        endif()

    else()
        message(STATUS "NOT Visual Studio")

        if(CMAKE_BUILD_TYPE STREQUAL "ReleaseQuadmath")
            set(CMAKE_CXX_EXTENSIONS TRUE) # -std=gnu++11 instead of -std=c++11
            message(STATUS "Linking with Quadmath library")
            target_link_libraries(sndspecLib ${SNDFILE_LIBRARY} libfftw3-3 libquadmath ${CMAKE_THREAD_LIBS_INIT})
        else()
            target_link_libraries(sndspecLib ${SNDFILE_LIBRARY} libfftw3-3 ${CMAKE_THREAD_LIBS_INIT})
        endif()

    endif()

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

add_executable(sndspec main.cpp)
target_link_libraries(sndspec sndspecLib)
