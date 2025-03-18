#!/bin/bash

# Exit on any error
set -e

echo "Building gtkks with bundled GTK3 libraries..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Running CMake..."
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
echo "Compiling with $CORES cores..."
make -j$CORES

# Create bundle directory
echo "Creating application bundle..."
mkdir -p gtkks.app/Contents/MacOS
mkdir -p gtkks.app/Contents/lib

# Copy executable
cp gtkks gtkks.app/Contents/MacOS/

# Bundle libraries
echo "Bundling libraries with dylibbundler..."
dylibbundler -od -b -x gtkks.app/Contents/MacOS/gtkks -d gtkks.app/Contents/lib/ -p @executable_path/../lib/

echo "Bundle complete! You can find the application at build/gtkks.app"
echo "Run with: ./build/gtkks.app/Contents/MacOS/gtkks" 