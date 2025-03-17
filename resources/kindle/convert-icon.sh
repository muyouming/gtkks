#!/bin/bash

# Convert SVG icon to PNG for Kindle
# Requires Inkscape or rsvg-convert

if command -v inkscape &> /dev/null; then
    echo "Using Inkscape to convert icon..."
    inkscape -w 256 -h 256 icon.svg -o icon.png
elif command -v rsvg-convert &> /dev/null; then
    echo "Using rsvg-convert to convert icon..."
    rsvg-convert -w 256 -h 256 icon.svg -o icon.png
else
    echo "Error: Neither Inkscape nor rsvg-convert found. Please install one of them."
    exit 1
fi

echo "Icon converted successfully to icon.png" 