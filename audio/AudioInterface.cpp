#include "AudioInterface.h"
#include "simulationworker.h"
#include "BufferManager.h"
#include <mutex>

// Constants
const unsigned int WIDTH = 256;
const unsigned int HEIGHT = 256;

extern SimulationWorker simWorker;  // Assuming simWorker is globally accessible 

// Use the mutex from simWorker directly
std::mutex *pBufferSwapMutex = nullptr;

// Global or static variables to keep track of the state
static float prev_avg = 0.0f;
static float next_avg = 0.0f;
static unsigned int frames_since_last_update = 0;

int audio_callback(void *outputBuffer, [[maybe_unused]] void *inputBuffer, unsigned int nBufferFrames,
                   [[maybe_unused]] double streamTime, [[maybe_unused]] RtAudioStreamStatus status, [[maybe_unused]] void *userData)
{
    uint8_t **current_state;
    {
        std::lock_guard<std::mutex> lock(*pBufferSwapMutex);
        current_state = simWorker.getReadState();  // Use the simWorker to get the readState
    }
    // Get the average audio data from a small domain.
    float *buffer = (float *)outputBuffer;
    unsigned int domainSize = 10; // Example domain size
    float sum = 0.0f;
    for (unsigned int i = HEIGHT / 2 - domainSize / 2; i < HEIGHT / 2 + domainSize / 2; ++i)
    {
        for (unsigned int j = WIDTH / 2 - domainSize / 2; j < WIDTH / 2 + domainSize / 2; ++j)
        {
            sum += current_state[i][j];
        }
    }
    next_avg = sum / (domainSize * domainSize) / 255.0f - 0.5f; // Normalize to [-0.5, 0.5]

    // Interpolate between prev_avg and next_avg
    for (unsigned int i = 0; i < nBufferFrames; ++i)
    {
        float alpha = (float)frames_since_last_update / nBufferFrames;
        buffer[i] = (1.0f - alpha) * prev_avg + alpha * next_avg;
        frames_since_last_update++;
    }

    // Once we've filled a full buffer since the last update, update the previous average
    if (frames_since_last_update >= nBufferFrames)
    {
        prev_avg = next_avg;
        frames_since_last_update = 0;
    }

    return 0;
}

void setupAudio() {
    // Initialize pBufferSwapMutex during audio setup
    pBufferSwapMutex = simWorker.getBufferSwapMutex();
}

void cleanupAudio()
{
    // Any cleanup related to the audio (if needed)
}
