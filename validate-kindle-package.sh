#!/bin/bash

# Script to validate a Kindle package

# Exit on error
set -e

echo "=== GTKKS Kindle Package Validator ==="

# Check if package exists
if [ ! -f "kindle-build/gtkks-kindle.tar.gz" ]; then
    echo "Error: Package not found at kindle-build/gtkks-kindle.tar.gz"
    exit 1
fi

# Create temp directory
TEMP_DIR=$(mktemp -d)
echo "Using temporary directory: $TEMP_DIR"

# Extract package to temp directory
echo "Extracting package..."
tar -xzf kindle-build/gtkks-kindle.tar.gz -C "$TEMP_DIR"

# Check if executable exists
echo "Checking for executable..."
if [ ! -f "$TEMP_DIR/gtkks-kindle/gtkks" ]; then
    echo "Error: Executable not found in package"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Check if executable has proper permissions
echo "Checking executable permissions..."
if [ ! -x "$TEMP_DIR/gtkks-kindle/gtkks" ]; then
    echo "Error: Executable does not have execute permissions"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Check file type
echo "Checking file type..."
file_type=$(file "$TEMP_DIR/gtkks-kindle/gtkks")
if [[ ! $file_type == *"executable"* ]]; then
    echo "Error: File is not a proper executable. Got: $file_type"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Test run the executable
echo "Test running executable..."
if ! "$TEMP_DIR/gtkks-kindle/gtkks" > /dev/null; then
    echo "Error: Executable test run failed"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Check if launcher script exists
echo "Checking for launcher script..."
if [ ! -f "$TEMP_DIR/gtkks-kindle/gtkks.sh" ]; then
    echo "Error: Launcher script not found in package"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Check if launcher script has proper permissions
echo "Checking launcher script permissions..."
if [ ! -x "$TEMP_DIR/gtkks-kindle/gtkks.sh" ]; then
    echo "Error: Launcher script does not have execute permissions"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Check if resources directory exists
echo "Checking for resources directory..."
if [ ! -d "$TEMP_DIR/gtkks-kindle/resources" ]; then
    echo "Error: Resources directory not found in package"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Check if icon exists
echo "Checking for icon..."
if [ ! -f "$TEMP_DIR/gtkks-kindle/resources/kindle/icon.svg" ]; then
    echo "Error: Icon not found in package"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Clean up
echo "Cleaning up..."
rm -rf "$TEMP_DIR"

echo "Validation successful! Package is valid."
echo "Package location: kindle-build/gtkks-kindle.tar.gz" 