#include "AudioInterface.h"
#include "AudioThread.h"
#include <rtaudio/RtAudio.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

std::atomic<bool> isAudioThreadRunning = true; 

void runAudioThread()
{
    std::cout << "Audio thread started." << std::endl;
    std::flush(std::cout);
    RtAudio dac;
    try
    {
        if (dac.getDeviceCount() < 1)
        {
            std::cerr << "\nNo audio devices found!\n";
            return; // Exit the thread gracefully
        }

        RtAudio::StreamParameters parameters;
        parameters.deviceId = dac.getDefaultOutputDevice();
        parameters.nChannels = 1;
        unsigned int sampleRate = 44100;
        unsigned int bufferFrames = 256; // 256 sample frames
        dac.openStream(&parameters, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &audio_callback, nullptr);
        dac.startStream();

        while (dac.isStreamRunning() && isAudioThreadRunning)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        dac.stopStream();
        if (dac.isStreamOpen())
        {
            dac.closeStream();
        }
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
}
