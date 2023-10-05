#include "AudioInterface.h"
#include "SimulationWorker.h"
#include "BufferManager.h"
#include <mutex>
#include <cmath>

const unsigned int WIDTH = 256;
const unsigned int HEIGHT = 256;

extern SimulationWorker simWorker;
std::mutex *pBufferSwapMutex = nullptr;

static float prev_avg = 0.0f;
static float next_avg = 0.0f;
static unsigned int frames_since_last_update = 0;

// Constants for generating a sine wave
const double frequency = 440.0; // Frequency for A4 note
const double twoPi = 2.0 * 3.141592653589793;
double timeCounter = 0.0; // Keeps track of the current time (renamed from "time" to "timeCounter")

// int audio_callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
//                    double streamTime, RtAudioStreamStatus status, void *userData)
// {
//     uint8_t **current_state;
//     {
//         std::lock_guard<std::mutex> lock(*pBufferSwapMutex);
//         current_state = simWorker.getReadState();
//     }

//     float *buffer = (float *)outputBuffer;
//     unsigned int domainSize = 10;

//     if (!current_state) {
//         for (unsigned int i = 0; i < nBufferFrames; ++i) {
//             buffer[i] = 0.0f;
//         }
//         return 0;
//     }

//     float sum = 0.0f;
//     for (unsigned int i = HEIGHT / 2 - domainSize / 2; i < HEIGHT / 2 + domainSize / 2; ++i) {
//         for (unsigned int j = WIDTH / 2 - domainSize / 2; j < WIDTH / 2 + domainSize / 2; ++j) {
//             sum += current_state[i][j];
//         }
//     }
//     next_avg = sum / (domainSize * domainSize) / 255.0f - 0.5f;

//     for (unsigned int i = 0; i < nBufferFrames; ++i) {
//         float alpha = (float)frames_since_last_update / nBufferFrames;
//         buffer[i] = (1.0f - alpha) * prev_avg + alpha * next_avg;
//         frames_since_last_update++;
//     }

//     if (frames_since_last_update >= nBufferFrames) {
//         prev_avg = next_avg;
//         frames_since_last_update = 0;
//     }

//     if (frames_since_last_update % 44100 == 0) {
//         std::cout << "Audio state: prev_avg = " << prev_avg << ", next_avg = " << next_avg << std::endl;
//         std::flush(std::cout);
//     }

//     return 0;
// }
int audio_callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                   double streamTime, RtAudioStreamStatus status, void *userData)
{
    float *buffer = (float *)outputBuffer;
    if (status)
        std::cerr << "Stream overflow detected!" << std::endl;

    // Generate and fill the buffer with sine wave samples
    for (unsigned int i = 0; i < nBufferFrames; i++)
    {
        buffer[i] = 0.5 * sin(twoPi * frequency * timeCounter); // 0.5 is the amplitude
        timeCounter += 1.0 / 44100.0;                           // Assuming a 44100 sample rate
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
