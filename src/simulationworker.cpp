#include "simulationworker.h"
#include <iostream>
#include <cstring>
SimulationWorker::SimulationWorker() : isDone(false)
{
    current_state = new uint8_t *[HEIGHT];
    next_state = new uint8_t *[HEIGHT];
    for (size_t i = 0; i < HEIGHT; i++)
    {
        current_state[i] = new uint8_t[WIDTH]();
        next_state[i] = new uint8_t[WIDTH]();
    }
    // Set up initial conditions
    for (int i = HEIGHT / 2 - 5; i < HEIGHT / 2 + 5; i++)
    {
        for (int j = WIDTH / 2 - 5; j < WIDTH / 2 + 5; j++)
        {
            current_state[i][j] = 255;
        }
    }
    readState = current_state;
    writeState = next_state;
}

void SimulationWorker::compute()
{
    isDone = false;
    workerThread = std::thread([this]()
                               {
        for (int step = 0; step < STEPS; step++) {
            if (step % 100 == 0) {
                std::cout << "Step: " << step << std::endl;
            }
#pragma omp parallel sections
            {
#pragma omp section
                compute_next_state(current_state, next_state, WIDTH, HEIGHT);

#pragma omp section
                apply_boundary_conditions(next_state, WIDTH, HEIGHT);

#pragma omp section
                add_sustained_excitation(next_state, WIDTH, HEIGHT, step);
            }

            // Copy the newly computed state to the writeState buffer
            for (size_t i = 0; i < HEIGHT; i++) {
                memcpy(writeState[i], next_state[i], WIDTH * sizeof(uint8_t));
            }

            // Swap buffers after computation
            {
                std::lock_guard<std::mutex> lock(bufferSwapMutex);
                std::swap(readState, writeState);
            }
        }

        std::unique_lock<std::mutex> lock(cv_mutex);
        isDone = true;
        cv.notify_one(); });
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
    return readState;
}

std::mutex *SimulationWorker::getBufferSwapMutex()
{
    return &bufferSwapMutex;
}

SimulationWorker::~SimulationWorker() {
    if (workerThread.joinable()) {
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
