#include "simulationworker.h"
#include <iostream>
#include <cstring>

SimulationWorker::SimulationWorker() : bufferManager(WIDTH, HEIGHT), isDone(false)
{
    uint8_t **currentState = bufferManager.getReadState();
    for (size_t i = HEIGHT / 2 - 5; i < HEIGHT / 2 + 5; i++)
    {
        for (size_t j = WIDTH / 2 - 5; j < WIDTH / 2 + 5; j++)
        {
            currentState[i][j] = 255;
        }
    }
}

void SimulationWorker::compute()
{
    uint8_t **current_state = bufferManager.getReadState();
    uint8_t **next_state = bufferManager.getWriteState(); 
    size_t step = 0;  // Initialize timestep

    while (!shouldExit) {  // Check the flag to exit the loop
        // Note: You might want to add an exit condition here based on some variable or flag.
        
        compute_next_state(current_state, next_state, WIDTH, HEIGHT);
        apply_boundary_conditions(next_state, WIDTH, HEIGHT);
        add_sustained_excitation(next_state, WIDTH, HEIGHT, step);

        // Copy the newly computed state to the writeState buffer
        for (size_t i = 0; i < HEIGHT; i++) {
            memcpy(next_state[i], current_state[i], WIDTH * sizeof(uint8_t));
        }

        // Swap buffers after computation
        bufferManager.swapBuffers();
        step++;  // Increment timestep
    }
}

void SimulationWorker::waitForCompletion()
{
    std::unique_lock<std::mutex> lock(cv_mutex);
    while (!isDone)
        cv.wait(lock);
    workerThread.join();
}

uint8_t **SimulationWorker::getReadState() const
{
    return bufferManager.getReadState();
}

uint8_t **SimulationWorker::getWriteState() const
{
    return bufferManager.getWriteState();
}

std::mutex *SimulationWorker::getBufferSwapMutex()
{
    return bufferManager.getMutex();
}

SimulationWorker::~SimulationWorker()
{
    if (workerThread.joinable())
    {
        workerThread.join();
    }
    // Cleanup code if any. For example:
    for (size_t i = 0; i < HEIGHT; i++)
    {
        delete[] current_state[i];
        delete[] next_state[i];
    }
    delete[] current_state;
    delete[] next_state;
}
