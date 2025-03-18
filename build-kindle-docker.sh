#!/bin/bash

# Build script for compiling GTKKS for Kindle using Docker

# Exit on error
set -e

echo "=== GTKKS Kindle Docker Build Script ==="

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed. Please install Docker first."
    exit 1
fi

# Create output directory
mkdir -p kindle-build

# Build Docker image
echo "Building Docker image..."
docker build -t gtkks-kindle-builder -f Dockerfile-real .

# Run Docker container to build the application
echo "Building application in Docker container..."
docker run --rm -v "$(pwd)/kindle-build:/output" gtkks-kindle-builder

# Check if build was successful
if [ -f "kindle-build/gtkks-kindle.tar.gz" ]; then
    echo "Build successful! The Kindle package is at kindle-build/gtkks-kindle.tar.gz"
    
    # Validate the package
    echo "Validating package..."
    mkdir -p kindle-build/temp
    tar -xzf kindle-build/gtkks-kindle.tar.gz -C kindle-build/temp
    
    if [ ! -f "kindle-build/temp/gtkks-kindle/gtkks" ]; then
        echo "Error: Executable not found in package"
        rm -rf kindle-build/temp
        exit 1
    fi
    
    if [ ! -x "kindle-build/temp/gtkks-kindle/gtkks" ]; then
        echo "Error: Executable does not have execute permissions"
        rm -rf kindle-build/temp
        exit 1
    fi
    
    echo "Package validation successful!"
    rm -rf kindle-build/temp
    
    # Copy installation script to the build directory
    cp kindle-install.sh kindle-build/
    
    echo ""
    echo "To install on your Kindle:"
    echo "1. Copy kindle-build/gtkks-kindle.tar.gz and kindle-build/kindle-install.sh to your Kindle"
    echo "2. SSH into your Kindle: ssh root@kindle-ip-address"
    echo "3. Run: cd /mnt/us && sh kindle-install.sh"
    echo "4. Restart your Kindle"
else
    echo "Build failed. Check the logs for errors."
    exit 1
fi 