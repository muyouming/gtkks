# GTKKS for Kindle - Build Instructions

This document provides instructions on how to build and install GTKKS (GTK LLM Client) for Kindle e-readers.

## Quick Start with Placeholder Package

For testing the installation process without full cross-compilation, you can use the placeholder package:

```bash
# Build the placeholder package
./build-kindle-placeholder.sh

# Validate the package (optional, already included in build)
./validate-kindle-package.sh

# Install the placeholder package on your Kindle
./install-on-kindle.sh
```

The placeholder package includes:
- A placeholder executable that displays a message when run
- A launcher script that sets up the environment
- An icon for the Kindle menu
- Installation scripts for the Kindle

This allows you to test the installation process and menu integration without needing to set up the full cross-compilation environment.

## Package Validation

All build scripts now include validation steps to ensure the executable is properly built:

1. The build script checks that:
   - The executable has proper permissions
   - The file is a proper binary executable
   - The executable runs successfully
   - The tarball contains all required files

2. A separate validation script is also provided:
   ```bash
   ./validate-kindle-package.sh
   ```
   This script performs comprehensive checks on an existing package.

## Full Cross-Compilation (Advanced)

Full cross-compilation for Kindle requires setting up a cross-compilation environment with the necessary ARM libraries. This is an advanced process and may require significant setup.

### Prerequisites

- Docker and Docker Compose (for the containerized build environment)
- ARM cross-compiler (gcc-arm-linux-gnueabi)
- GTK3 libraries compiled for ARM

### Using Docker (Recommended)

We provide a Docker-based build environment to simplify the cross-compilation process:

```bash
# Build using Docker
./build-with-docker.sh
```

Note: The Docker build process is still under development and may encounter issues with library dependencies.

### Manual Cross-Compilation

If you want to set up the cross-compilation environment manually, you'll need:

1. ARM cross-compiler (e.g., `gcc-arm-linux-gnueabi`)
2. GTK3 libraries compiled for ARM
3. CURL and JSON libraries compiled for ARM
4. CMake with a proper toolchain file

The `kindle-toolchain.cmake` file provides a starting point for the CMake toolchain configuration.

## Installing on Kindle

### Automatic Installation

The easiest way to install is using the provided script:

```bash
./install-on-kindle.sh
```

This script will:
1. Prompt for your Kindle's IP address
2. Copy the necessary files to your Kindle
3. Execute the installation script on the Kindle
4. Create a menu entry with an icon

### Manual Installation

If you prefer to install manually:

1. Copy the package to your Kindle:
   ```bash
   scp kindle-build/gtkks-kindle.tar.gz root@kindle-ip-address:/mnt/us/
   scp kindle-install.sh root@kindle-ip-address:/mnt/us/
   ```

2. SSH into your Kindle:
   ```bash
   ssh root@kindle-ip-address
   ```

3. Run the installation script:
   ```bash
   cd /mnt/us
   sh kindle-install.sh
   ```

4. Restart your Kindle to see the GTKKS application in the menu.

## Troubleshooting

### Docker Build Issues

- **Permission denied error**: Make sure the build scripts are executable:
  ```bash
  chmod +x build-kindle.sh build-with-docker.sh
  ```

- **Missing libraries**: The Docker build may fail due to missing ARM libraries. This is a known limitation of the current build process.

### Placeholder Package

- If you're using the placeholder package and encounter issues, check that:
  - The placeholder executable is included in the package
  - The installation script has the correct permissions
  - Your Kindle is accessible via SSH

## Resources

- [MobileRead Forum: Compiling GTK native app for Kindle Touch](https://www.mobileread.com/forums/showthread.php?t=189372)
- [KOReader: Cross-compilation toolchain for Kindle](https://github.com/koreader/koxtoolchain) 