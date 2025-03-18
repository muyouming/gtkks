#!/bin/bash

# Exit on error
set -e

echo "=== GTKKS Kindle Installation Script ==="

# Check if the package exists
if [ ! -f "kindle-build/gtkks-kindle.tar.gz" ]; then
    echo "Error: Package not found at kindle-build/gtkks-kindle.tar.gz"
    echo "Please run ./build-kindle-docker.sh first to build the package."
    exit 1
fi

# Prompt for Kindle IP address
echo -n "Enter your Kindle's IP address: "
read kindle_ip

# Check if IP address is empty
if [ -z "$kindle_ip" ]; then
    echo "Error: No IP address provided."
    exit 1
fi

# Check if scp and ssh commands are available
if ! command -v scp &> /dev/null || ! command -v ssh &> /dev/null; then
    echo "Error: scp and/or ssh commands not found. Please install OpenSSH."
    exit 1
fi

echo "Copying files to Kindle at $kindle_ip..."
scp kindle-build/gtkks-kindle.tar.gz kindle-build/kindle-install.sh root@$kindle_ip:/mnt/us/

echo "Installing GTKKS on Kindle..."
ssh root@$kindle_ip "cd /mnt/us && sh kindle-install.sh"

echo "Installation complete! Please restart your Kindle to see GTKKS in the menu." 