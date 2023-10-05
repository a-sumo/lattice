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
#include "Application.h"
using namespace wgpu;

const size_t WIDTH = 256;
const size_t HEIGHT = 256;
const size_t STEPS = 44100;


const double targetFrameTime = 1.0 / 60.0; // 60 FPS
double lastTime = glfwGetTime();

#define UNUSED(x) (void)x;

// Double buffering
uint8_t **currentStateBuffer1;
uint8_t **currentStateBuffer2;
uint8_t **readState = nullptr;  // Used by audio_callback
uint8_t **writeState = nullptr; // Updated by automaton

std::mutex bufferSwapMutex; // Used to protect buffer swap operations

int main(int, char **)
{
	Application app;
	if (!app.onInit())
		return 1;
	SimulationWorker simWorker;
	// Seed the simulation
	setupAudio(simWorker.getReadState(), simWorker.getBufferSwapMutex());
	std::thread audioThread(runAudioThread);

	while (app.isRunning())
	{
		simWorker.compute();
		simWorker.waitForCompletion();

		double currentTime = glfwGetTime();
		if (currentTime - lastTime >= targetFrameTime)
		{
			app.onFrame();
			glfwPollEvents();
			lastTime = currentTime;
		}
	}

	audioThread.join();
	cleanupAudio();

	app.onFinish();
	return 0;
}

