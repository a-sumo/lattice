#include "Application.h"
#include "automaton_2d.h"
#include "audio/AudioThread.h"
#include "audio/AudioInterface.h"
#include "simulationworker.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <glfw3webgpu.h>
#include <cassert>
#include <vector>
using namespace wgpu;

// const size_t WIDTH = 256;
// const size_t HEIGHT = 256;
// const size_t STEPS = 44100;
const double targetFrameTime = 1.0 / 60.0; // 60 FPS
double lastTime = glfwGetTime();

#define UNUSED(x) (void)x;

SimulationWorker simWorker;

int main(int, char **)
{
    Application app;
    if (!app.onInit())
        return 1;

    setupAudio();  // We removed the parameters since we're now using the global simWorker

	std::thread simulationThread([&]()
	{
		while (app.isRunning()) {
			simWorker.compute();
		}
		simWorker.requestExit();  // Request simulation worker to exit its loop
	});

	std::thread audioThread(runAudioThread);

    while (app.isRunning())
    {
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= targetFrameTime)
        {
            std::cout << "Frame time: " << currentTime - lastTime << std::endl;
            app.onFrame();
            glfwPollEvents();
            lastTime = currentTime;
        }
    }
	isAudioThreadRunning = false;  // Set the flag to false to exit the audio thread loop
    // Stop audio and simulation worker before joining threads.
    // Add mechanisms in runAudioThread and simWorker.compute to break out of their loops when the app is not running.
    audioThread.join();
    simulationThread.join();

    cleanupAudio();
    app.onFinish();

    return 0;
}

