cmake_minimum_required(VERSION 3.1...3.25)
project(Lattice VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Debug configuration by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum OS X deployment version")

# Fallback paths for RtAudio
set(RTAUDIO_INCLUDE_DIR "/opt/homebrew/opt/rtaudio/include" CACHE PATH "RtAudio include directory")
set(RTAUDIO_LIBRARY "/opt/homebrew/opt/rtaudio/lib/librtaudio.dylib" CACHE FILEPATH "RtAudio library path")

# Debug flags
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -Xclang -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Xclang -fopenmp")

# Source files
set(SOURCES
    src/main.cpp
    src/automaton_2d.cpp
    src/webgpu-utils.cpp
<<<<<<< Updated upstream
    src/audio_handler.cpp
=======
    src/Application.cpp
    src/ResourceManager.cpp
    src/implementations.cpp
    src/simulationworker.cpp
    audio/AudioInterface.cpp
    audio/AudioThread.cpp
>>>>>>> Stashed changes
)

# OpenMP paths
set(OPENMP_LIB_PATH "/opt/homebrew/opt/libomp/lib/libomp.dylib")
set(OPENMP_INCLUDE_PATH "/opt/homebrew/opt/libomp/include")

# Create executable
<<<<<<< Updated upstream
add_executable(lattice ${SOURCES})
=======
add_executable(lattice 
include/Application.h
include/ResourceManager.h
include/simulationworker.h
${SOURCES})
>>>>>>> Stashed changes

target_include_directories(lattice PRIVATE 
    ${RTAUDIO_INCLUDE_DIR} 
    ${OPENMP_INCLUDE_PATH} 
    audio
    include
    external
)

# Linking libraries
target_link_libraries(lattice PRIVATE 
    ${RTAUDIO_LIBRARY} 
    ${OPENMP_LIB_PATH}
    glfw3webgpu
    webgpu
)

# Compiler settings
set_target_properties(lattice PROPERTIES COMPILE_WARNINGS_AS_ERROR ON)
if (MSVC)
    target_compile_options(lattice PRIVATE /W4)
else()
    target_compile_options(lattice PRIVATE -Wall -Wextra -pedantic)
endif()

# XCode settings for macOS
if(XCODE)
    set_target_properties(lattice PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal")
endif()

# Adding necessary subdirectories
add_subdirectory(glfw)
add_subdirectory(webgpu)
add_subdirectory(glfw3webgpu)

# If using the wgpu-native distribution with precompiled binaries
target_copy_webgpu_binaries(lattice)
