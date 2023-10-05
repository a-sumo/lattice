#ifndef SIMULATIONWORKER_H
#define SIMULATIONWORKER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include "automaton_2d.h"

class SimulationWorker
{
public:
    SimulationWorker();
    ~SimulationWorker();
    void compute();
    void waitForCompletion();
    std::mutex* getBufferSwapMutex();
    uint8_t** getReadState() const;
private:
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
};

#endif
