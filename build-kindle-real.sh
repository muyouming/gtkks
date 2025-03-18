#!/bin/bash

# Build script for compiling GTKKS for Kindle using the real source code

# Exit on error
set -e

echo "=== GTKKS Kindle Real Build Script ==="

# Create build directory
mkdir -p kindle-build

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is not installed. Please install CMake first."
    exit 1
fi

# Check if required libraries are installed
echo "Checking for required libraries..."
pkg-config --exists gtkmm-3.0 || { echo "Error: gtkmm-3.0 not found"; exit 1; }
pkg-config --exists curlpp || { echo "Error: curlpp not found"; exit 1; }
pkg-config --exists jsoncpp || { echo "Error: jsoncpp not found"; exit 1; }

# Copy the Kindle-specific CMakeLists if it exists
if [ -f "CMakeLists-kindle.txt" ]; then
    echo "Using Kindle-specific CMakeLists.txt..."
    cp CMakeLists-kindle.txt kindle-build/CMakeLists.txt
else
    echo "Using standard CMakeLists.txt..."
    cp CMakeLists.txt kindle-build/CMakeLists.txt
fi

# Copy source files to build directory
echo "Copying source files..."
cp -r src kindle-build/
cp -r include kindle-build/
cp -r resources kindle-build/

# Configure and build the project
echo "Configuring and building the project..."
cd kindle-build

# Determine number of CPU cores for parallel build
if [[ "$(uname)" == "Darwin" ]]; then
    # macOS
    CORES=$(sysctl -n hw.ncpu)
else
    # Linux
    CORES=$(nproc)
fi

# Configure with CMake
cmake . -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building GTKKS for Kindle..."
make -j$CORES

# Check if build was successful
if [ ! -f "gtkks" ]; then
    echo "Error: Build failed. Executable not found."
    exit 1
fi

# Validate the executable
echo "Validating executable..."
if [ ! -x "gtkks" ]; then
    echo "Error: Failed to create executable with proper permissions"
    exit 1
fi

# Check file type to ensure it's a binary executable
file_type=$(file gtkks)
if [[ ! $file_type == *"executable"* ]]; then
    echo "Error: File is not a proper executable. Got: $file_type"
    exit 1
fi

echo "Executable validation successful"

# Create package directory
echo "Creating Kindle package..."
mkdir -p gtkks-kindle
cp gtkks gtkks-kindle/

# Create launcher script
cat > gtkks-kindle/gtkks.sh << 'EOF'
#!/bin/sh
cd "$(dirname "$0")"
export LD_LIBRARY_PATH=/usr/lib:/lib:/opt/lib
echo 1 > /proc/eink_fb/update_display
./gtkks
EOF

chmod +x gtkks-kindle/gtkks.sh

# Copy resources
echo "Copying resources..."
mkdir -p gtkks-kindle/resources
if [ -d "resources" ]; then
    cp -r resources/* gtkks-kindle/resources/
fi

# Create Kindle-specific resources if they don't exist
mkdir -p gtkks-kindle/resources/kindle
if [ ! -f "gtkks-kindle/resources/kindle/icon.svg" ]; then
    cat > gtkks-kindle/resources/kindle/icon.svg << 'EOF'
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg xmlns="http://www.w3.org/2000/svg" width="256" height="256" viewBox="0 0 256 256">
  <rect width="256" height="256" fill="white" rx="32" ry="32"/>
  <g transform="translate(32, 32)">
    <path d="M 96 16 C 44 16 0 48 0 96 C 0 144 44 176 96 176 C 100 176 104 176 108 176 L 160 192 L 144 144 C 172 128 192 112 192 96 C 192 48 148 16 96 16 z" fill="black"/>
    <text x="96" y="104" font-family="Sans" font-size="48" text-anchor="middle" fill="white">LLM</text>
  </g>
</svg>
EOF
fi

# Create a README file
cat > gtkks-kindle/README.txt << 'EOF'
GTKKS for Kindle
================

This is GTKKS, a GTK-based LLM client for Kindle.

To run the application:
1. Execute the gtkks.sh script
2. Enter your prompts in the application interface
3. Use the menu to select different models and settings

For more information, please refer to the project repository.
EOF

# Create package
echo "Creating tarball..."
tar -czvf gtkks-kindle.tar.gz gtkks-kindle/

# Validate the tarball
echo "Validating tarball contents..."
if ! tar -tvf gtkks-kindle.tar.gz | grep -q "gtkks-kindle/gtkks"; then
    echo "Error: Executable not found in tarball"
    exit 1
fi

echo "Build complete. Kindle package is at kindle-build/gtkks-kindle.tar.gz" 