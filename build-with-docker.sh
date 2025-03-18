#!/bin/bash

# Wrapper script to build GTKKS for Kindle using Docker

# Exit on error
set -e

echo "=== GTKKS Kindle Docker Build Script ==="

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed. Please install Docker first."
    exit 1
fi

# Check if Docker Compose is installed
if ! command -v docker-compose &> /dev/null; then
    echo "Error: Docker Compose is not installed. Please install Docker Compose first."
    exit 1
fi

# Create build directory
mkdir -p kindle-build

# Build the Docker image and run the build
echo "Building Docker image and running build..."
docker-compose up --build

# Check if build was successful
if [ -f "kindle-build/gtkks-kindle.tar.gz" ]; then
    echo "Build successful! The Kindle package is at kindle-build/gtkks-kindle.tar.gz"
    
    # Validate the package
    echo "Validating package..."
    if [ -f "./validate-kindle-package.sh" ]; then
        ./validate-kindle-package.sh
        if [ $? -ne 0 ]; then
            echo "Error: Package validation failed. The build may be incomplete."
            exit 1
        fi
        echo "Package validation successful!"
    else
        echo "Warning: Validation script not found. Skipping validation."
    fi
    
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