#include "AudioInterface.h"
#include "SimulationWorker.h"
#include "BufferManager.h"
#include <mutex>

const unsigned int WIDTH = 256;
const unsigned int HEIGHT = 256;

extern SimulationWorker simWorker; 
std::mutex *pBufferSwapMutex = nullptr;

static float prev_avg = 0.0f;
static float next_avg = 0.0f;
static unsigned int frames_since_last_update = 0;

int audio_callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                   double streamTime, RtAudioStreamStatus status, void *userData)
{
    uint8_t **current_state;
    {
        std::lock_guard<std::mutex> lock(*pBufferSwapMutex);
        current_state = simWorker.getReadState();
    }

    float *buffer = (float *)outputBuffer;
    unsigned int domainSize = 10;

    if (!current_state) {
        for (unsigned int i = 0; i < nBufferFrames; ++i) {
            buffer[i] = 0.0f;
        }
        return 0;
    }

    float sum = 0.0f;
    for (unsigned int i = HEIGHT / 2 - domainSize / 2; i < HEIGHT / 2 + domainSize / 2; ++i) {
        for (unsigned int j = WIDTH / 2 - domainSize / 2; j < WIDTH / 2 + domainSize / 2; ++j) {
            sum += current_state[i][j];
        }
    }
    next_avg = sum / (domainSize * domainSize) / 255.0f - 0.5f;

    for (unsigned int i = 0; i < nBufferFrames; ++i) {
        float alpha = (float)frames_since_last_update / nBufferFrames;
        buffer[i] = (1.0f - alpha) * prev_avg + alpha * next_avg;
        frames_since_last_update++;
    }

    if (frames_since_last_update >= nBufferFrames) {
        prev_avg = next_avg;
        frames_since_last_update = 0;
    }

    if (frames_since_last_update % 44100 == 0) {
        std::cout << "Audio state: prev_avg = " << prev_avg << ", next_avg = " << next_avg << std::endl;
        std::flush(std::cout);
    }

    return 0;
}

void setupAudio()
{
    pBufferSwapMutex = simWorker.getBufferSwapMutex();
}

void cleanupAudio()
{
}
