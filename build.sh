#!/bin/bash

<<<<<<< Updated upstream
=======
# Check for backend argument, default to WGPU if none provided
BACKEND=${1:-DAWN}

>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
./build/lattice
=======
RUST_BACKTRACE=full ./build/lattice 
>>>>>>> Stashed changes
