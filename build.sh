#!/bin/bash

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Determine number of CPU cores for parallel build
if [[ "$(uname)" == "Darwin" ]]; then
    # macOS
    CORES=$(sysctl -n hw.ncpu)
else
    # Linux
    CORES=$(nproc)
fi

# Build
make -j$CORES

echo "Build complete. Run with ./build/gtkks" 