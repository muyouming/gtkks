#!/bin/bash

# Build script for cross-compiling GTKKS for Kindle
# Based on information from: https://www.mobileread.com/forums/showthread.php?t=189372

# Exit on error
set -e

# Create build directory
mkdir -p kindle-build

# Set up cross-compilation environment
export KINDLE_TC=/opt/kindle-tc
export PATH=$KINDLE_TC/bin:$PATH
export CROSS_COMPILE=arm-kindle-linux-gnueabi-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export CFLAGS="-O2 -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp"
export CXXFLAGS="$CFLAGS"
export PKG_CONFIG_PATH=$KINDLE_TC/arm-kindle-linux-gnueabi/sysroot/usr/lib/pkgconfig

# Configure with CMake
cd kindle-build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../kindle-toolchain.cmake \
    -DCMAKE_INSTALL_PREFIX=/opt/amazon/gtkks \
    -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Create Kindle package
mkdir -p gtkks-kindle
cp gtkks gtkks-kindle/
cp -r ../resources gtkks-kindle/

# Create launcher script
cat > gtkks-kindle/gtkks.sh << 'EOF'
#!/bin/sh
cd "$(dirname "$0")"
export LD_LIBRARY_PATH=/usr/lib:/lib:/opt/lib
echo 1 > /proc/eink_fb/update_display
./gtkks
EOF

chmod +x gtkks-kindle/gtkks.sh

# Create package
tar -czvf gtkks-kindle.tar.gz gtkks-kindle/

echo "Build complete. Kindle package is at kindle-build/gtkks-kindle.tar.gz" 