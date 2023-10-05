#include "SimulationWorker.h"
#include <iostream>
#include <cstring>
#include "stb_image_write.h"

SimulationWorker::SimulationWorker() : bufferManager(WIDTH, HEIGHT), isDone(false)
{
    uint8_t **currentState = bufferManager.getReadState();
    if(!currentState) return;
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
    if(!current_state || !next_state) {
        std::cerr << "Failed to get simulation buffers." << std::endl;
        return;
    }

    size_t step = 0;
    size_t debugMaxSteps = 10000;
    while (!shouldExit && step < debugMaxSteps)
    {
        compute_next_state(current_state, next_state, WIDTH, HEIGHT);
        apply_boundary_conditions(next_state, WIDTH, HEIGHT);
        add_sustained_excitation(next_state, WIDTH, HEIGHT, step);

        bufferManager.swapBuffers();

        // Update the state pointers after the swap
        current_state = bufferManager.getReadState();
        next_state = bufferManager.getWriteState();

        step++;

        if (step % 100 == 0)
        {
            std::cout << "Simulation step: " << step << std::endl;
        }

        if (step % 1000 == 0)
        {
            char filename[50];
            sprintf(filename, "state_%zu.png", step);
            uint8_t *flat_data = new uint8_t[WIDTH * HEIGHT];
            for (size_t i = 0; i < HEIGHT; i++)
            {
                for (size_t j = 0; j < WIDTH; j++)
                {
                    flat_data[i * WIDTH + j] = current_state[i][j];
                }
            }
            stbi_write_png(filename, WIDTH, HEIGHT, 1, flat_data, WIDTH);
            delete[] flat_data;

            // Logging the audio buffer for debugging
            float sum = 0.0f;
            for (size_t i = 0; i < HEIGHT; i++)
            {
                for (size_t j = 0; j < WIDTH; j++)
                {
                    sum += current_state[i][j];
                }
            }
            float avg = sum / (WIDTH * HEIGHT);
            std::cout << "Average audio buffer value at step " << step << ": " << avg << std::endl;
        }
    }

    std::cout << "Simulation thread exited the loop after " << step << " steps. ShouldExit: " << shouldExit << std::endl;
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
}
