#!/bin/bash

# Exit on error
set -e

echo "=== GTKKS Kindle Scribe (armhf) Build Script ==="

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed. Please install Docker first."
    exit 1
fi

# Create output directory
mkdir -p kindle-build

# Build the Docker image
echo "Building Docker image for armhf cross-compilation..."
docker build -t gtkks-kindle-armhf-builder -f Dockerfile-armhf .

# Run the Docker container to build the application
echo "Building application in Docker container..."
docker run --rm -v "$(pwd)/kindle-build:/output" gtkks-kindle-armhf-builder

# Check if build was successful
if [ -f "kindle-build/gtkks-kindle-armhf.tar.gz" ]; then
    echo "Build successful! The Kindle Scribe package is at kindle-build/gtkks-kindle-armhf.tar.gz"
    
    # Validate the package
    echo "Validating package..."
    mkdir -p kindle-build/validation
    tar -xzf kindle-build/gtkks-kindle-armhf.tar.gz -C kindle-build/validation
    
    if [ -f "kindle-build/validation/gtkks-kindle/gtkks" ]; then
        # Check if it's an ARM executable
        file_info=$(file kindle-build/validation/gtkks-kindle/gtkks)
        if [[ $file_info == *"ARM"* ]]; then
            echo "Validation successful! The package contains a valid ARM executable."
            
            # Copy installation script to the build directory
            cp kindle-build/kindle-install.sh kindle-build/kindle-install-armhf.sh
            sed -i.bak 's/gtkks-kindle.tar.gz/gtkks-kindle-armhf.tar.gz/g' kindle-build/kindle-install-armhf.sh
            
            # Create an installation script for the armhf version
            cat > install-on-kindle-armhf.sh << 'EOF'
#!/bin/bash

# Exit on error
set -e

echo "=== GTKKS Kindle Scribe (armhf) Installation Script ==="

# Check if the package exists
if [ ! -f "kindle-build/gtkks-kindle-armhf.tar.gz" ]; then
    echo "Error: Package not found at kindle-build/gtkks-kindle-armhf.tar.gz"
    echo "Please run ./build-kindle-armhf.sh first to build the package."
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
            
            echo ""
            echo "To install on your Kindle Scribe:"
            echo "1. Run: ./install-on-kindle-armhf.sh"
            echo "2. Enter your Kindle Scribe's IP address when prompted"
            echo "3. Restart your Kindle Scribe after installation"
        else
            echo "Error: The executable is not an ARM binary. Cross-compilation may have failed."
            exit 1
        fi
    else
        echo "Error: The executable is missing from the package."
        exit 1
    fi
    
    # Clean up validation directory
    rm -rf kindle-build/validation
else
    echo "Build failed. Check the logs for errors."
    exit 1
fi 