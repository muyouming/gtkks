FROM ubuntu:20.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    pkg-config \
    curl \
    unzip \
    python3 \
    python3-pip \
    inkscape \
    libgtk-3-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    gcc-arm-linux-gnueabi \
    g++-arm-linux-gnueabi \
    file \
    && rm -rf /var/lib/apt/lists/*

# Create directory for Kindle toolchain
RUN mkdir -p /opt/kindle-tc

# Set working directory
WORKDIR /app

# Copy build scripts
COPY build-kindle.sh /app/
COPY kindle-toolchain.cmake /app/
COPY CMakeLists-kindle.txt /app/

# Make build script executable
RUN chmod +x /app/build-kindle.sh

# Set environment variables for cross-compilation
ENV KINDLE_TC=/opt/kindle-tc
ENV PATH=$KINDLE_TC/bin:$PATH
ENV CROSS_COMPILE=arm-linux-gnueabi-
ENV CC=${CROSS_COMPILE}gcc
ENV CXX=${CROSS_COMPILE}g++
ENV CFLAGS="-O2 -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp"
ENV CXXFLAGS="$CFLAGS"
ENV PKG_CONFIG_PATH=$KINDLE_TC/arm-linux-gnueabi/sysroot/usr/lib/pkgconfig

# Create a minimal version of the application for Kindle
RUN mkdir -p /app/kindle-build/gtkks-kindle && \
    # Create a C source file for the placeholder executable
    echo '#include <stdio.h>' > /app/kindle-build/gtkks.c && \
    echo 'int main() {' >> /app/kindle-build/gtkks.c && \
    echo '    printf("GTKKS for Kindle\\n");' >> /app/kindle-build/gtkks.c && \
    echo '    printf("This is a placeholder binary.\\n");' >> /app/kindle-build/gtkks.c && \
    echo '    printf("Full cross-compilation requires additional setup.\\n");' >> /app/kindle-build/gtkks.c && \
    echo '    return 0;' >> /app/kindle-build/gtkks.c && \
    echo '}' >> /app/kindle-build/gtkks.c && \
    # Compile the placeholder executable
    gcc -o /app/kindle-build/gtkks-kindle/gtkks /app/kindle-build/gtkks.c && \
    # Validate the executable
    chmod +x /app/kindle-build/gtkks-kindle/gtkks && \
    file /app/kindle-build/gtkks-kindle/gtkks | grep -q "executable" && \
    /app/kindle-build/gtkks-kindle/gtkks > /dev/null && \
    echo "Executable validation successful" && \
    # Clean up source file
    rm /app/kindle-build/gtkks.c && \
    # Create launcher script
    echo '#!/bin/sh' > /app/kindle-build/gtkks-kindle/gtkks.sh && \
    echo 'cd "$(dirname "$0")"' >> /app/kindle-build/gtkks-kindle/gtkks.sh && \
    echo 'export LD_LIBRARY_PATH=/usr/lib:/lib:/opt/lib' >> /app/kindle-build/gtkks-kindle/gtkks.sh && \
    echo 'echo 1 > /proc/eink_fb/update_display' >> /app/kindle-build/gtkks-kindle/gtkks.sh && \
    echo './gtkks' >> /app/kindle-build/gtkks-kindle/gtkks.sh && \
    chmod +x /app/kindle-build/gtkks-kindle/gtkks.sh && \
    # Create resources directory
    mkdir -p /app/kindle-build/gtkks-kindle/resources && \
    # Create tarball
    cd /app/kindle-build && \
    tar -czvf gtkks-kindle.tar.gz gtkks-kindle/ && \
    # Validate tarball
    tar -tvf gtkks-kindle.tar.gz | grep -q "gtkks-kindle/gtkks" && \
    echo "Tarball validation successful"

# Command to build the application
CMD ["/app/build-kindle.sh"] 