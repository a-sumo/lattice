#ifndef AUDIO_INTERFACE_H
#define AUDIO_INTERFACE_H

#include <rtaudio/RtAudio.h>
#include <cstdint>
extern std::mutex* pBufferSwapMutex;

// Function prototype for the audio callback
int audio_callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                   double streamTime, RtAudioStreamStatus status, void *userData);

// Setup and cleanup functions
void setupAudio(uint8_t **readBuffer, std::mutex *bufferMutex);
void cleanupAudio();

#endif
