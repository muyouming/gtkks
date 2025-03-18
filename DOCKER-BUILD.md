# Building GTKKS for Kindle using Docker

This document explains how to build GTKKS (GTK LLM Client) for Kindle e-readers using Docker, which simplifies the cross-compilation process by providing a consistent build environment.

## Prerequisites

- Docker installed on your system
- Docker Compose installed on your system
- Internet connection (to download the toolchain and dependencies)

## Quick Start

The easiest way to build GTKKS for Kindle is to use the provided wrapper script:

```bash
./build-with-docker.sh
```

This script will:
1. Check if Docker and Docker Compose are installed
2. Create a Docker container with the necessary build environment
3. Download the Kindle cross-compilation toolchain and dependencies
4. Build GTKKS for Kindle
5. Create a package ready for installation on your Kindle

## Manual Build Process

If you prefer to run the Docker commands manually, follow these steps:

1. Build the Docker image:
   ```bash
   docker-compose build
   ```

2. Run the build process:
   ```bash
   docker-compose up
   ```

3. The build output will be in the `kindle-build` directory, specifically:
   - `kindle-build/gtkks-kindle.tar.gz`: The packaged application
   - `kindle-build/gtkks`: The compiled binary

## Installing on Kindle

After building the application:

1. Copy the following files to your Kindle:
   - `kindle-build/gtkks-kindle.tar.gz`
   - `kindle-install.sh`

2. SSH into your Kindle:
   ```bash
   ssh root@kindle-ip-address
   # Default password is usually 'mario'
   ```

3. Run the installation script:
   ```bash
   cd /mnt/us
   sh kindle-install.sh
   ```

4. Restart your Kindle.

5. After restart, you should see GTKKS in the Kindle menu.

## Troubleshooting

- **Docker build fails**: Check if you have enough disk space and that Docker has permission to access the necessary directories.
- **Toolchain download fails**: The script uses fallback URLs from the KOReader project. If these are outdated, you may need to find and specify alternative sources.
- **GTK dependencies missing**: The build script attempts to download GTK dependencies automatically. If this fails, you may need to manually obtain and install them.
- **Application doesn't run on Kindle**: Check the logs at `/var/log/messages` on your Kindle for error information.

## Customizing the Build

If you need to customize the build process:

- Edit `Dockerfile` to modify the build environment
- Edit `build-kindle.sh` to change build parameters or steps
- Edit `kindle-toolchain.cmake` to adjust cross-compilation settings 