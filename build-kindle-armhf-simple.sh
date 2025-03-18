#!/bin/bash

# Exit on error
set -e

echo "=== GTKKS Kindle Scribe (armhf) Simple Build Script ==="

# Create build directory
mkdir -p kindle-build/gtkks-kindle

# Create a simple C executable
echo "Creating placeholder executable with logging functionality..."
cat > kindle-build/gtkks-main.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>

#define LOG_FILE "gtkks_log.txt"
#define MAX_PATH 1024

FILE *log_file = NULL;

void log_message(const char *message) {
    if (log_file) {
        time_t now;
        struct tm *timeinfo;
        char timestamp[30];
        
        time(&now);
        timeinfo = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        fprintf(log_file, "[%s] %s\n", timestamp, message);
        fflush(log_file);
    }
}

void log_user_input(const char *input) {
    if (log_file) {
        char buffer[1100];
        snprintf(buffer, sizeof(buffer), "User input: %s", input);
        log_message(buffer);
    }
}

void log_system_response(const char *response) {
    if (log_file) {
        char buffer[1100];
        snprintf(buffer, sizeof(buffer), "System response: %s", response);
        log_message(buffer);
    }
}

void initialize_logging(const char *exec_path) {
    char log_path[MAX_PATH];
    char dir_path[MAX_PATH];
    
    // Get the directory of the executable
    strncpy(dir_path, exec_path, sizeof(dir_path));
    char *dir = dirname(dir_path);
    
    // Create the log file path
    snprintf(log_path, sizeof(log_path), "%s/%s", dir, LOG_FILE);
    
    // Open the log file
    log_file = fopen(log_path, "a");
    if (!log_file) {
        fprintf(stderr, "Error opening log file: %s\n", strerror(errno));
        return;
    }
    
    log_message("=== GTKKS Application Started ===");
    log_message("Logging initialized");
}

void close_logging() {
    if (log_file) {
        log_message("=== GTKKS Application Closed ===");
        fclose(log_file);
        log_file = NULL;
    }
}

void display_welcome() {
    printf("\n");
    printf("  ██████╗ ████████╗██╗  ██╗██╗  ██╗███████╗\n");
    printf(" ██╔════╝ ╚══██╔══╝██║ ██╔╝██║ ██╔╝██╔════╝\n");
    printf(" ██║  ███╗   ██║   █████╔╝ █████╔╝ ███████╗\n");
    printf(" ██║   ██║   ██║   ██╔═██╗ ██╔═██╗ ╚════██║\n");
    printf(" ╚██████╔╝   ██║   ██║  ██╗██║  ██╗███████║\n");
    printf("  ╚═════╝    ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝\n");
    printf("\n");
    printf("Welcome to GTKKS for Kindle Scribe (armhf)\n");
    printf("A GTK-based LLM client\n");
    printf("\n");
    
    log_message("Welcome message displayed");
}

void process_input() {
    char input[1024];
    char response[1024];
    
    printf("Enter your message (or 'exit' to quit):\n> ");
    log_message("Waiting for user input");
    
    while (fgets(input, sizeof(input), stdin)) {
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        log_user_input(input);
        
        if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            log_message("User requested exit");
            break;
        }
        
        printf("\nProcessing your message: \"%s\"\n", input);
        printf("Simulating response from GPT-4...\n\n");
        log_message("Processing user message");
        
        // Simulate thinking
        printf("Thinking");
        for (int i = 0; i < 3; i++) {
            fflush(stdout);
            sleep(1);
            printf(".");
        }
        printf("\n\n");
        
        // Generate a simple response
        if (strstr(input, "hello") || strstr(input, "hi")) {
            strcpy(response, "Hello! How can I assist you today on your Kindle Scribe?");
            printf("%s\n", response);
        } else if (strstr(input, "help")) {
            strcpy(response, "This is a placeholder for the GTKKS application. In the full version, you would be able to:\n1. Chat with various LLM models\n2. Configure API settings\n3. Save and load conversations\n4. Customize the interface");
            printf("%s\n", response);
        } else if (strstr(input, "kindle") || strstr(input, "scribe")) {
            strcpy(response, "Yes, this application is designed specifically for the Kindle Scribe with armhf architecture.\nIt provides a lightweight interface for interacting with language models.");
            printf("%s\n", response);
        } else if (strstr(input, "log") || strstr(input, "logs")) {
            strcpy(response, "Yes, this application automatically creates logs in its root folder.\nCheck the gtkks_log.txt file for the complete log history.");
            printf("%s\n", response);
        } else {
            snprintf(response, sizeof(response), "I understand you're interested in \"%s\". In the full version of GTKKS,\nI would provide a more detailed and helpful response based on your query.\nThis is just a placeholder to demonstrate the application structure.", input);
            printf("%s\n", response);
        }
        
        log_system_response(response);
        
        printf("\nEnter your message (or 'exit' to quit):\n> ");
        log_message("Waiting for user input");
    }
}

int main(int argc, char *argv[]) {
    // Initialize logging with the executable path
    initialize_logging(argv[0]);
    
    // Log system information
    log_message("System: Kindle Scribe (armhf)");
    
    display_welcome();
    process_input();
    
    close_logging();
    return 0;
}
EOF

# Compile using Docker with Ubuntu x86_64 and cross-compiler
echo "Compiling with Docker and cross-compiler..."
docker run --rm -v "$(pwd)/kindle-build:/work" \
    -w /work \
    ubuntu:20.04 \
    bash -c "apt-get update && \
             apt-get install -y gcc-arm-linux-gnueabihf && \
             arm-linux-gnueabihf-gcc -o gtkks-kindle/gtkks gtkks-main.c -static"

# Check if the executable was created
if [ ! -f "kindle-build/gtkks-kindle/gtkks" ]; then
    echo "Error: Failed to create the armhf executable."
    exit 1
fi

# Verify it's an ARM executable
file_info=$(file kindle-build/gtkks-kindle/gtkks)
if [[ ! $file_info == *"ARM"* ]]; then
    echo "Error: The executable is not an ARM binary. Cross-compilation failed."
    echo "File info: $file_info"
    exit 1
fi

echo "Successfully created ARM executable for Kindle Scribe."

# Create launcher script
echo "Creating launcher script..."
cat > kindle-build/gtkks-kindle/gtkks.sh << 'EOF'
#!/bin/sh
cd "$(dirname "$0")"
./gtkks
EOF
chmod +x kindle-build/gtkks-kindle/gtkks.sh

# Copy resources
echo "Copying resources..."
mkdir -p kindle-build/gtkks-kindle/resources/kindle
if [ -d "resources/kindle" ]; then
    cp -r resources/kindle/* kindle-build/gtkks-kindle/resources/kindle/
else
    # Create a simple icon if resources don't exist
    echo '<svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 48 48"><circle cx="24" cy="24" r="20" fill="none" stroke="black" stroke-width="2"/><text x="24" y="28" text-anchor="middle" font-family="sans-serif" font-size="12">GTKKS</text></svg>' > kindle-build/gtkks-kindle/resources/kindle/icon.svg
fi

# Create README
echo "Creating README..."
cat > kindle-build/gtkks-kindle/README.md << 'EOF'
# GTKKS for Kindle Scribe (armhf)

This is the GTKKS application for Kindle Scribe with armhf architecture.

To run the application, execute the gtkks.sh script.

## Features

- Simple text-based interface
- Placeholder for the full GTK application
- Optimized for Kindle Scribe
- Automatic logging to gtkks_log.txt in the application directory

## Usage

Just run the `gtkks.sh` script and follow the prompts.

## Logs

The application automatically creates a log file named `gtkks_log.txt` in its root folder.
This log file contains information about application startup, user inputs, and system responses.
EOF

# Create package
echo "Creating package..."
cd kindle-build
tar -czvf gtkks-kindle-armhf.tar.gz gtkks-kindle/

# Create installation script
echo "Creating installation script..."
cat > kindle-install-armhf.sh << 'EOF'
#!/bin/sh

# GTKKS Kindle Scribe Installation Script
# This script should be run on the Kindle device

echo "=== GTKKS Kindle Scribe (armhf) Installation Script ==="

# Check if we're running on a Kindle
if [ ! -d "/mnt/us" ]; then
    echo "Error: This script must be run on a Kindle device."
    exit 1
fi

# Create installation directories
echo "Creating installation directories..."
mkdir -p /mnt/us/gtkks-kindle
mkdir -p /mnt/us/extensions/gtkks/bin
mkdir -p /mnt/us/extensions/gtkks/icons

# Extract application files
echo "Extracting application files..."
if [ -f "gtkks-kindle-armhf.tar.gz" ]; then
    tar -xzvf gtkks-kindle-armhf.tar.gz -C /mnt/us/
else
    echo "Error: gtkks-kindle-armhf.tar.gz not found."
    exit 1
fi

# Create desktop shortcut
echo "Creating desktop shortcut..."
cat > /mnt/us/extensions/gtkks/bin/gtkks.sh << 'INNEREOF'
#!/bin/sh
cd /mnt/us/gtkks-kindle
./gtkks.sh
INNEREOF

chmod +x /mnt/us/extensions/gtkks/bin/gtkks.sh

# Create menu item
echo "Creating menu item..."
cat > /mnt/us/extensions/gtkks/menu.json << 'INNEREOF'
{
    "items": [
        {
            "name": "GTKKS",
            "priority": 1,
            "action": "/mnt/us/extensions/gtkks/bin/gtkks.sh"
        }
    ]
}
INNEREOF

# Create icon
echo "Creating icon..."
if [ -f "/mnt/us/gtkks-kindle/resources/kindle/icon.svg" ]; then
    cp /mnt/us/gtkks-kindle/resources/kindle/icon.svg /mnt/us/extensions/gtkks/icons/gtkks.svg
fi

echo "Installation complete. Restart your Kindle to see GTKKS in the menu."
EOF

# Create installation script for the host
cd ..
cat > install-on-kindle-armhf.sh << 'EOF'
#!/bin/bash

# Exit on error
set -e

echo "=== GTKKS Kindle Scribe (armhf) Installation Script ==="

# Check if the package exists
if [ ! -f "kindle-build/gtkks-kindle-armhf.tar.gz" ]; then
    echo "Error: Package not found at kindle-build/gtkks-kindle-armhf.tar.gz"
    echo "Please run ./build-kindle-armhf-simple.sh first to build the package."
    exit 1
fi

# Prompt for Kindle IP address
echo -n "Enter your Kindle Scribe's IP address: "
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

echo "Copying files to Kindle Scribe at $kindle_ip..."
scp kindle-build/gtkks-kindle-armhf.tar.gz kindle-build/kindle-install-armhf.sh root@$kindle_ip:/mnt/us/

echo "Installing GTKKS on Kindle Scribe..."
ssh root@$kindle_ip "cd /mnt/us && sh kindle-install-armhf.sh"

echo "Installation complete! Please restart your Kindle Scribe to see GTKKS in the menu."
EOF

chmod +x install-on-kindle-armhf.sh

echo "Build successful! The Kindle Scribe package is at kindle-build/gtkks-kindle-armhf.tar.gz"
echo ""
echo "To install on your Kindle Scribe:"
echo "1. Run: ./install-on-kindle-armhf.sh"
echo "2. Enter your Kindle Scribe's IP address when prompted"
echo "3. Restart your Kindle Scribe after installation" 