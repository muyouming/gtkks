#!/bin/bash

# Script to build GTKKS for Kindle and automatically install it on a Kindle device

# Exit on error
set -e

echo "=== GTKKS Kindle Build and Install Script ==="

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
if [ ! -f "kindle-build/gtkks-kindle.tar.gz" ]; then
    echo "Build failed. Check the logs for errors."
    exit 1
fi

echo "Build successful! The Kindle package is at kindle-build/gtkks-kindle.tar.gz"
echo "Note: This is a placeholder package. Full cross-compilation requires additional setup."

# Copy installation script to the build directory
cp kindle-install.sh kindle-build/

# Ask for Kindle IP address
read -p "Enter your Kindle's IP address (or press Enter to skip installation): " kindle_ip

if [ -z "$kindle_ip" ]; then
    echo "Installation skipped."
    echo ""
    echo "To install manually on your Kindle:"
    echo "1. Copy kindle-build/gtkks-kindle.tar.gz and kindle-build/kindle-install.sh to your Kindle"
    echo "2. SSH into your Kindle: ssh root@kindle-ip-address"
    echo "3. Run: cd /mnt/us && sh kindle-install.sh"
    echo "4. Restart your Kindle"
    exit 0
fi

# Check if scp and ssh are available
if ! command -v scp &> /dev/null || ! command -v ssh &> /dev/null; then
    echo "Error: scp and/or ssh commands not found. Please install OpenSSH client."
    exit 1
fi

# Copy files to Kindle
echo "Copying files to Kindle at $kindle_ip..."
scp kindle-build/gtkks-kindle.tar.gz kindle-build/kindle-install.sh root@$kindle_ip:/mnt/us/

# Run installation script on Kindle
echo "Running installation script on Kindle..."
ssh root@$kindle_ip "cd /mnt/us && sh kindle-install.sh"

echo "Installation complete! Please restart your Kindle to see GTKKS in the menu." 