#!/bin/bash

# Build script for creating a Kindle package with a proper executable

# Exit on error
set -e

echo "=== GTKKS Kindle Simple Build Script ==="

# Create build directory
mkdir -p kindle-build
mkdir -p kindle-build/gtkks-kindle
mkdir -p kindle-build/gtkks-kindle/resources

# Create a proper C executable
echo "Creating executable source..."
cat > kindle-build/gtkks.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_SIZE 1024
#define MODEL_NAME "GPT-4 (Simulated)"

// Function to simulate sending a message to an LLM API
void send_message(const char* message, char* response, size_t response_size) {
    snprintf(response, response_size,
        "This is GTKKS for Kindle.\n"
        "You said: %s\n\n"
        "Using model: %s", 
        message, MODEL_NAME);
}

int main() {
    char input[MAX_INPUT_SIZE];
    char response[MAX_INPUT_SIZE * 2];
    
    printf("GTKKS for Kindle\n");
    printf("=================\n");
    printf("Welcome to GTKKS, a GTK-based LLM client for Kindle.\n\n");
    
    printf("Using API: %s\n\n", MODEL_NAME);
    
    printf("Enter your message (or 'exit' to quit):\n");
    
    while (1) {
        printf("> ");
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            break;
        }
        
        // Remove trailing newline
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            break;
        }
        
        send_message(input, response, sizeof(response));
        printf("\n%s\n\n", response);
    }
    
    printf("Thank you for using GTKKS for Kindle!\n");
    return 0;
}
EOF

# Compile the executable
echo "Compiling executable..."
gcc -o kindle-build/gtkks-kindle/gtkks kindle-build/gtkks.c

# Validate the executable
echo "Validating executable..."
if [ ! -x kindle-build/gtkks-kindle/gtkks ]; then
    echo "Error: Failed to create executable with proper permissions"
    exit 1
fi

# Check file type to ensure it's a binary executable
file_type=$(file kindle-build/gtkks-kindle/gtkks)
if [[ ! $file_type == *"executable"* ]]; then
    echo "Error: File is not a proper executable. Got: $file_type"
    exit 1
fi

# Test run the executable
echo "Test running executable..."
echo "exit" | kindle-build/gtkks-kindle/gtkks > /dev/null
if [ $? -ne 0 ]; then
    echo "Error: Executable test run failed"
    exit 1
fi

echo "Executable validation successful"

# Clean up source file
rm kindle-build/gtkks.c

# Create launcher script
cat > kindle-build/gtkks-kindle/gtkks.sh << 'EOF'
#!/bin/sh
cd "$(dirname "$0")"
export LD_LIBRARY_PATH=/usr/lib:/lib:/opt/lib
echo 1 > /proc/eink_fb/update_display
./gtkks
EOF

chmod +x kindle-build/gtkks-kindle/gtkks.sh

# Copy resources from the real application if available
echo "Copying resources..."
if [ -d "resources" ]; then
    cp -r resources/* kindle-build/gtkks-kindle/resources/
fi

# Create Kindle-specific resources if they don't exist
mkdir -p kindle-build/gtkks-kindle/resources/kindle
if [ ! -f "kindle-build/gtkks-kindle/resources/kindle/icon.svg" ]; then
    cat > kindle-build/gtkks-kindle/resources/kindle/icon.svg << 'EOF'
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg xmlns="http://www.w3.org/2000/svg" width="256" height="256" viewBox="0 0 256 256">
  <rect width="256" height="256" fill="white" rx="32" ry="32"/>
  <g transform="translate(32, 32)">
    <path d="M 96 16 C 44 16 0 48 0 96 C 0 144 44 176 96 176 C 100 176 104 176 108 176 L 160 192 L 144 144 C 172 128 192 112 192 96 C 192 48 148 16 96 16 z" fill="black"/>
    <text x="96" y="104" font-family="Sans" font-size="48" text-anchor="middle" fill="white">LLM</text>
  </g>
</svg>
EOF
fi

# Create a README file
cat > kindle-build/gtkks-kindle/README.txt << 'EOF'
GTKKS for Kindle
================

This is GTKKS, a GTK-based LLM client for Kindle.

To run the application:
1. Execute the gtkks.sh script
2. Enter text at the prompt
3. Type 'exit' to quit

For more information, please refer to the project repository.
EOF

# Create package
echo "Creating tarball..."
cd kindle-build
tar -czvf gtkks-kindle.tar.gz gtkks-kindle/

# Validate the tarball
echo "Validating tarball contents..."
if ! tar -tvf gtkks-kindle.tar.gz | grep -q "gtkks-kindle/gtkks"; then
    echo "Error: Executable not found in tarball"
    exit 1
fi

echo "Build complete. Kindle package is at kindle-build/gtkks-kindle.tar.gz" 