#!/bin/bash

# Script to install the GTKKS package on a Kindle

# Exit on error
set -e

echo "=== GTKKS Kindle Installation Script ==="

# Check if the package exists
if [ ! -f "kindle-build/gtkks-kindle.tar.gz" ]; then
    echo "Error: Package not found at kindle-build/gtkks-kindle.tar.gz"
    echo "Please run ./build-kindle-simple.sh first."
    exit 1
fi

# Ask for Kindle IP address
read -p "Enter your Kindle's IP address: " kindle_ip

if [ -z "$kindle_ip" ]; then
    echo "Error: No IP address provided."
    exit 1
fi

# Check if scp and ssh are available
if ! command -v scp &> /dev/null || ! command -v ssh &> /dev/null; then
    echo "Error: scp and/or ssh commands not found. Please install OpenSSH client."
    exit 1
fi

# Copy files to Kindle
echo "Copying files to Kindle at $kindle_ip..."
scp kindle-build/gtkks-kindle.tar.gz kindle-install.sh root@$kindle_ip:/mnt/us/

# Run installation script on Kindle
echo "Running installation script on Kindle..."
ssh root@$kindle_ip "cd /mnt/us && sh kindle-install.sh"

echo "Installation complete! Please restart your Kindle to see GTKKS in the menu." 