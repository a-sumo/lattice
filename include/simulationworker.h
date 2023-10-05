#ifndef SIMULATIONWORKER_H
#define SIMULATIONWORKER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include "automaton_2d.h"
#include "BufferManager.h"

class SimulationWorker
{
public:
    SimulationWorker();
    ~SimulationWorker();
    void compute();
    void waitForCompletion();
    std::mutex* getBufferSwapMutex();
    uint8_t** getReadState() const;
    uint8_t** getWriteState() const;
    void requestExit() { shouldExit = true; } 
private:
    BufferManager bufferManager;
    static const size_t WIDTH = 256;
    static const size_t HEIGHT = 256;
    static const size_t STEPS = 44100;
    uint8_t **current_state;
    uint8_t **next_state;
    bool isDone;
    std::thread workerThread;
    std::condition_variable cv;
    std::mutex cv_mutex;
    uint8_t **readState;
    uint8_t **writeState;
    std::mutex bufferSwapMutex;
    bool shouldExit = false;  // Exit flag to control the loop in compute()
};

#endif
