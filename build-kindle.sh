#!/bin/bash

# Build script for cross-compiling GTKKS for Kindle
# Based on information from: https://www.mobileread.com/forums/showthread.php?t=189372

# Exit on error
set -e

echo "=== GTKKS Kindle Build Script ==="

# Create build directory
mkdir -p kindle-build

# Set up cross-compilation environment
export CROSS_COMPILE=arm-linux-gnueabi-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export CFLAGS="-O2 -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp"
export CXXFLAGS="$CFLAGS"

# Check if cross-compiler is available
if ! command -v ${CC} &> /dev/null; then
    echo "Error: ARM cross-compiler not found. Please install gcc-arm-linux-gnueabi."
    exit 1
fi

echo "Using cross-compiler: $(${CC} --version | head -n 1)"

# Copy the simplified CMakeLists.txt file
if [ -f "CMakeLists-kindle.txt" ]; then
    echo "Using simplified CMakeLists.txt for Kindle..."
    cp CMakeLists-kindle.txt CMakeLists.txt
fi

# Convert SVG icon to PNG
echo "Converting SVG icon to PNG..."
mkdir -p resources/kindle
if [ -f "resources/kindle/icon.svg" ]; then
    if command -v inkscape &> /dev/null; then
        # Check Inkscape version
        INKSCAPE_VERSION=$(inkscape --version | awk '{print $2}' | cut -d. -f1)
        if [ "$INKSCAPE_VERSION" -ge "1" ]; then
            # Inkscape 1.0 or newer
            inkscape --export-filename=resources/kindle/icon.png -w 256 -h 256 resources/kindle/icon.svg
        else
            # Inkscape 0.9x or older
            inkscape -z -e resources/kindle/icon.png -w 256 -h 256 resources/kindle/icon.svg
        fi
    elif command -v rsvg-convert &> /dev/null; then
        rsvg-convert -w 256 -h 256 resources/kindle/icon.svg -o resources/kindle/icon.png
    else
        echo "Warning: Neither Inkscape nor rsvg-convert found. Icon conversion skipped."
    fi
else
    echo "Warning: Icon SVG not found. Icon conversion skipped."
fi

# Configure with CMake
echo "Configuring with CMake..."
cd kindle-build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../kindle-toolchain.cmake \
    -DCMAKE_INSTALL_PREFIX=/opt/amazon/gtkks \
    -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building GTKKS for Kindle..."
make -j$(nproc)

# Create Kindle package
echo "Creating Kindle package..."
mkdir -p gtkks-kindle
cp gtkks gtkks-kindle/ || echo "Warning: Binary not found. Build may have failed."
cp -r ../resources gtkks-kindle/

# Create launcher script
echo "Creating launcher script..."
cat > gtkks-kindle/gtkks.sh << 'EOF'
#!/bin/sh
cd "$(dirname "$0")"
export LD_LIBRARY_PATH=/usr/lib:/lib:/opt/lib
echo 1 > /proc/eink_fb/update_display
./gtkks
EOF

chmod +x gtkks-kindle/gtkks.sh

# Create package
echo "Creating tarball..."
tar -czvf gtkks-kindle.tar.gz gtkks-kindle/

echo "Build complete. Kindle package is at kindle-build/gtkks-kindle.tar.gz" 