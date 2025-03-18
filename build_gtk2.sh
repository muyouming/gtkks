#!/bin/bash

# Exit on error
set -e

# Create build directory
mkdir -p build_gtk2
cd build_gtk2

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

echo "Build completed successfully!"
echo "You can run the GTK2 application with: ./gtkks"

# Return to original directory
cd .. 