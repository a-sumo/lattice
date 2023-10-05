#ifndef AUDIO_THREAD_H
#define AUDIO_THREAD_H

void runAudioThread();
extern std::atomic<bool> isAudioThreadRunning; 
#endif
