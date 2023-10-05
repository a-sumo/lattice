cmake_minimum_required(VERSION 3.1...3.25)
project(Lattice VERSION 0.1.0 LANGUAGES CXX C)

include(FetchContent)

FetchContent_Declare(
  webgpu
  GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
  GIT_TAG        main
)
FetchContent_MakeAvailable(webgpu)


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
    src/Application.cpp
    src/ResourceManager.cpp
    src/implementations.cpp
    src/SimulationWorker.cpp
    audio/AudioInterface.cpp
    audio/AudioThread.cpp
)

# OpenMP paths
set(OPENMP_LIB_PATH "/opt/homebrew/opt/libomp/lib/libomp.dylib")
set(OPENMP_INCLUDE_PATH "/opt/homebrew/opt/libomp/include")

# Create executable
add_executable(lattice 
include/BufferManager.h
include/stb_image.h
include/stb_image_write.h
${SOURCES})

# we add "." here to make sure that we can include "glm/glm.hpp" instead of "glm.hpp"
target_include_directories(lattice PRIVATE
    .
    ${RTAUDIO_INCLUDE_DIR} 
    ${OPENMP_INCLUDE_PATH} 
    audio
    include
)

# Linking libraries
target_link_libraries(lattice PRIVATE 
    ${RTAUDIO_LIBRARY} 
    ${OPENMP_LIB_PATH}
    # glfw
    glfw3webgpu
    webgpu
    imgui
)
if(DEV_MODE)
	# In dev mode, we load resources from the source tree, so that when we
	# dynamically edit resources (like shaders), these are correctly
	# versionned.
	target_compile_definitions(lattice PRIVATE
		RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
	)
else()
	# In release mode, we just load resources relatively to wherever the
	# executable is launched from, so that the binary is portable
	target_compile_definitions(lattice PRIVATE
		RESOURCE_DIR="./resources"
	)
endif()

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
# add_subdirectory(glfw)
add_subdirectory(webgpu)
add_subdirectory(glfw3webgpu)
add_subdirectory(imgui)

# If using the wgpu-native distribution with precompiled binaries
target_copy_webgpu_binaries(lattice)