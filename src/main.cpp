#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "automaton_2d.h"
#include "audio_handler.h"
#include "stb_image_write.h"
#include <iostream>
#include <mutex>
#include "webgpu-utils.h"
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <glfw3webgpu.h>
#include <cassert>
#include <vector>

#define WIDTH 256
#define HEIGHT 256
#define STEPS 44100

#define UNUSED(x) (void)x;

// Double buffering
uint8_t **currentStateBuffer1;
uint8_t **currentStateBuffer2;
uint8_t **readState = nullptr;  // Used by audio_callback
uint8_t **writeState = nullptr; // Updated by automaton

std::mutex bufferSwapMutex; // Used to protect buffer swap operations

int main(int, char **)
{
    // 1. Create a descriptor for WebGPU instance
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr;

    // 2. Create the WebGPU instance using the descriptor
    WGPUInstance instance = wgpuCreateInstance(&desc);

    // 3. Check the created instance
    if (!instance)
    {
        std::cerr << "Could not initialize WebGPU!" << std::endl;
        return 1;
    }

    // 4. Display the WebGPU instance (for debugging purposes)
    std::cout << "WGPU instance: " << instance << std::endl;
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return 1;
    }
    // Create the GLFW window
    GLFWwindow *window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);
    if (!window)
    {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }
    WGPUSurface surface = glfwGetWGPUSurface(instance, window);

    // Adapter options: we need the adapter to draw to the window's surface
    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.nextInChain = nullptr;
    adapterOpts.compatibleSurface = surface;

    // Get the adapter
    WGPUAdapter adapter = requestAdapter(instance, &adapterOpts);

    std::cout << "Got adapter: " << adapter << std::endl;

    std::cout << "Requesting device..." << std::endl;

    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    deviceDesc.label = "My Device";       // anything works here, that's your call
    deviceDesc.requiredFeaturesCount = 0; // we do not require any specific feature
    deviceDesc.requiredLimits = nullptr;  // we do not require any specific limit
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = "The default queue";
    WGPUDevice device = requestDevice(adapter, &deviceDesc);

    std::cout << "Got device: " << device << std::endl;
    // Add a callback that gets executed upon errors in our use of the device
    auto onDeviceError = [](WGPUErrorType type, char const *message, void * /* pUserData */)
    {
        std::cout << "Uncaptured device error: type " << type;
        if (message)
            std::cout << " (" << message << ")";
        std::cout << std::endl;
    };

    wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);

    // Get the main and only command queue used to send instructions to the GPU
    WGPUQueue queue = wgpuDeviceGetQueue(device);

#ifdef WEBGPU_BACKEND_DAWN
    // Add a callback to monitor the moment queued work winished
    auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void * /* pUserData */)
    {
        std::cout << "Queued work finished with status: " << status << std::endl;
    };
    wgpuQueueOnSubmittedWorkDone(queue, 0 /* non standard argument for Dawn */, onQueueWorkDone, nullptr /* pUserData */);
#endif // WEBGPU_BACKEND_DAWN

    // We create a command encoder to be able to create command buffers
    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "My command encoder";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

    // Encode some mock commands
    wgpuCommandEncoderInsertDebugMarker(encoder, "Do one thing");
    wgpuCommandEncoderInsertDebugMarker(encoder, "Do another thing");

    // Encode commands into a command buffer
    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);

    // Finally submit the command queue
    std::cout << "Submitting command..." << std::endl;
    wgpuQueueSubmit(queue, 1, &command);

    uint8_t **current_state = (uint8_t **)malloc(HEIGHT * sizeof(uint8_t *));
    uint8_t **next_state = (uint8_t **)malloc(HEIGHT * sizeof(uint8_t *));
    for (size_t i = 0; i < HEIGHT; i++)
    {
        current_state[i] = (uint8_t *)calloc(WIDTH, sizeof(uint8_t));
        next_state[i] = (uint8_t *)calloc(WIDTH, sizeof(uint8_t));
    }

    // Seed the simulation with a more pronounced pulse
    for (int i = HEIGHT / 2 - 5; i < HEIGHT / 2 + 5; i++)
    {
        for (int j = WIDTH / 2 - 5; j < WIDTH / 2 + 5; j++)
        {
            current_state[i][j] = 255;
        }
    }
    currentStateBuffer1 = (uint8_t **)malloc(HEIGHT * sizeof(uint8_t *));
    currentStateBuffer2 = (uint8_t **)malloc(HEIGHT * sizeof(uint8_t *));
    for (size_t i = 0; i < HEIGHT; i++)
    {
        currentStateBuffer1[i] = (uint8_t *)calloc(WIDTH, sizeof(uint8_t));
        currentStateBuffer2[i] = (uint8_t *)calloc(WIDTH, sizeof(uint8_t));
    }

    readState = currentStateBuffer1;
    writeState = currentStateBuffer2;
    // Initialize RtAudio and stream audio
    RtAudio dac;
    if (dac.getDeviceCount() < 1)
    {
        std::cout << "\nNo audio devices found!\n";
        exit(0);
    }

    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = 1;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 256; // 256 sample frames

    try
    {
        dac.openStream(&parameters, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &audio_callback, (void *)readState); // Corrected argument
        dac.startStream();
    }
    catch (RtAudioErrorType &e)
    {
        std::string errorMessage;
        switch (e)
        {
        case RTAUDIO_WARNING:
            errorMessage = "Warning!";
            break;
        case RTAUDIO_UNKNOWN_ERROR:
            errorMessage = "Unknown Error!";
            break;
        case RTAUDIO_NO_DEVICES_FOUND:
            errorMessage = "No devices found!";
            break;
        case RTAUDIO_INVALID_DEVICE:
            errorMessage = "Invalid device!";
            break;
        default:
            errorMessage = "Unhandled RtAudio error!";
        }
        std::cerr << "Failed to start audio stream: " << errorMessage << std::endl;
        exit(-1);
    }

    for (int step = 0; step < STEPS; step++)
    {
        if (step % 100 == 0)
        {
            std::cout << "Step: " << step << std::endl;
        }
        // Check for window close event
        if (glfwWindowShouldClose(window))
        {
            break;
        }

#pragma omp parallel sections
        {

#pragma omp section
            compute_next_state(current_state, next_state, WIDTH, HEIGHT);

#pragma omp section
            apply_boundary_conditions(current_state, WIDTH, HEIGHT);

#pragma omp section
            add_sustained_excitation(current_state, WIDTH, HEIGHT, step);
        }
        // Copy the newly computed state to the writeState buffer
        for (size_t i = 0; i < HEIGHT; i++)
        {
            memcpy(writeState[i], next_state[i], WIDTH * sizeof(uint8_t));
        }

        // Swap buffers after computation
        {
            std::lock_guard<std::mutex> lock(bufferSwapMutex);
            std::swap(readState, writeState);
        }
        glfwPollEvents();
    }

    // Stop and close the audio stream
    try
    {
        dac.stopStream();
        if (dac.isStreamOpen())
            dac.closeStream();
    }
    catch (RtAudioErrorType &e)
    {
        std::string errorMessage;
        switch (e)
        {
        case RTAUDIO_WARNING:
            errorMessage = "Warning!";
            break;
        case RTAUDIO_UNKNOWN_ERROR:
            errorMessage = "Unknown Error!";
            break;
        case RTAUDIO_NO_DEVICES_FOUND:
            errorMessage = "No devices found!";
            break;
        case RTAUDIO_INVALID_DEVICE:
            errorMessage = "Invalid device!";
            break;
        default:
            errorMessage = "Unhandled RtAudio error!";
        }
        std::cerr << "Failed to stop audio stream: " << errorMessage << std::endl;
        exit(-1);
    }

    for (size_t i = 0; i < HEIGHT; i++)
    { // Use size_t for the loop variable to fix the warning
        free(current_state[i]);
        free(next_state[i]);
    }
    free(current_state);
    free(next_state);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    // Don't forget to release the adapter
    wgpuDeviceRelease(device);
    wgpuSurfaceRelease(surface);
    wgpuAdapterRelease(adapter);
    wgpuInstanceRelease(instance);

    // Destroy the GLFW window before exiting
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
// int main (int, char**) {
// 	// We create the equivalent of the navigator.gpu if this were web code

// 	// 1. We create a descriptor
// 	WGPUInstanceDescriptor desc = {};
// 	desc.nextInChain = nullptr;

// 	// 2. We create the instance using this descriptor
// 	WGPUInstance instance = wgpuCreateInstance(&desc);

// 	// 3. We can check whether there is actually an instance created
// 	if (!instance) {
// 		std::cerr << "Could not initialize WebGPU!" << std::endl;
// 		return 1;
// 	}

// 	// 4. Display the object (WGPUInstance is a simple pointer, it may be
// 	// copied around without worrying about its size).
// 	std::cout << "WGPU instance: " << instance << std::endl;

// 	if (!glfwInit()) {
// 		std::cerr << "Could not initialize GLFW!" << std::endl;
// 		return 1;
// 	}

// 	GLFWwindow* window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);
// 	if (!window) {
// 		std::cerr << "Could not open window!" << std::endl;
// 		glfwTerminate();
// 		return 1;
// 	}

// 	while (!glfwWindowShouldClose(window)) {
// 		glfwPollEvents();
// 	}

// 	// 5. We clean up the WebGPU instance
// 	wgpuInstanceRelease(instance);

// 	glfwDestroyWindow(window);
// 	glfwTerminate();

// 	return 0;
// }