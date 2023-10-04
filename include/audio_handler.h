#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include <rtaudio/RtAudio.h>


int audio_callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                   double streamTime, RtAudioStreamStatus status, void *userData);

#endif
