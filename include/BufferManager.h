#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <mutex>

class BufferManager
{
private:
    uint8_t **readState;
    uint8_t **writeState;
    mutable std::mutex bufferSwapMutex;
    size_t width;
    size_t height;

public:
    BufferManager(size_t width, size_t height) : width(width), height(height)
    {
        readState = new uint8_t *[height];
        writeState = new uint8_t *[height];
        for (size_t i = 0; i < height; i++)
        {
            readState[i] = new uint8_t[width]();
            writeState[i] = new uint8_t[width]();
        }
    }

    ~BufferManager()
    {
        for (size_t i = 0; i < height; i++)
        {
            delete[] readState[i];
            delete[] writeState[i];
        }
        delete[] readState;
        delete[] writeState;
    }

    // Remove the BufferManager:: prefix here
    uint8_t **getReadState() const
    {
        std::lock_guard<std::mutex> lock(bufferSwapMutex);
        return readState;
    }

    uint8_t **getWriteState() const 
    {
        std::lock_guard<std::mutex> lock(bufferSwapMutex);
        return writeState;
    }

    void swapBuffers()
    {
        std::lock_guard<std::mutex> lock(bufferSwapMutex);
        std::swap(readState, writeState);
    }

    std::mutex *getMutex()
    {
        return &bufferSwapMutex;
    }

    // Delete copy constructor and copy assignment operator
    BufferManager(const BufferManager &) = delete;
    BufferManager &operator=(const BufferManager &) = delete;
};

#endif // BUFFERMANAGER_H
