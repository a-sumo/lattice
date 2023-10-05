#!/bin/bash

# Check for backend argument, default to WGPU if none provided
BACKEND=${1:-DAWN}

# Delete cache from build if exists
if [ -d "build" ]; then
    echo "Deleting cache from build..."
    rm -rf build/cache
fi

# Run CMake commands
echo "Running CMake with backend: $BACKEND..."
cmake . -B build -DWEBGPU_BACKEND=$BACKEND
cmake --build build

# Launch the binary with backtrace enabled
echo "Launching the application..."
lldb -- ./build/lattice
