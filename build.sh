#!/bin/bash

# Delete cache from build if exists
if [ -d "build" ]; then
    echo "Deleting cache from build..."
    rm -rf build/cache
fi

# Run CMake commands
echo "Running CMake..."
cmake . -B build
cmake --build build

# Launch the binary
echo "Launching the application..."
./build/lattice
