# GTKKS for Kindle

This document provides instructions for cross-compiling GTKKS (GTK LLM Client) for Kindle e-readers and installing it on your device.

## Prerequisites

To cross-compile for Kindle, you need to set up a cross-compilation toolchain. This process is based on information from [this MobileRead forum thread](https://www.mobileread.com/forums/showthread.php?t=189372).

### Setting up the Cross-Compilation Environment

1. Install the required packages on your development machine:

```bash
sudo apt-get install build-essential cmake git
```

2. Download and set up the Kindle cross-compilation toolchain:

```bash
# Create directory for the toolchain
sudo mkdir -p /opt/kindle-tc
sudo chown $USER /opt/kindle-tc

# Download the toolchain (example URL, you may need to find the appropriate toolchain)
wget https://www.mobileread.com/forums/attachment.php?attachmentid=116175 -O kindle-tc.tar.gz

# Extract the toolchain
tar -xzvf kindle-tc.tar.gz -C /opt/kindle-tc
```

3. Install GTK and other dependencies for the toolchain:

```bash
# These packages need to be cross-compiled for the Kindle or obtained from Kindle firmware
# This is a simplified example and may need adjustment
cd /opt/kindle-tc
wget https://www.mobileread.com/forums/attachment.php?attachmentid=116176 -O kindle-gtk.tar.gz
tar -xzvf kindle-gtk.tar.gz -C /opt/kindle-tc/arm-kindle-linux-gnueabi/sysroot/usr
```

## Building GTKKS for Kindle

1. Clone the GTKKS repository:

```bash
git clone https://github.com/yourusername/gtkks.git
cd gtkks
```

2. Run the Kindle build script:

```bash
chmod +x build-kindle.sh
./build-kindle.sh
```

This will create a package file `kindle-build/gtkks-kindle.tar.gz` that contains the compiled application.

## Installing on Kindle

1. Connect your Kindle to your computer via USB.

2. Copy the following files to your Kindle:
   - `kindle-build/gtkks-kindle.tar.gz`
   - `kindle-install.sh`

3. Safely eject your Kindle from your computer.

4. On your Kindle, enable the developer mode:
   - Open the search bar by tapping the top of the screen
   - Type `~ds`
   - In the developer settings, enable "Allow SSH connections"

5. Connect to your Kindle via SSH:

```bash
ssh root@192.168.15.244  # Replace with your Kindle's IP address
# Default password is usually 'mario'
```

6. Run the installation script:

```bash
cd /mnt/us
sh kindle-install.sh
```

7. Restart your Kindle.

8. After restart, you should see GTKKS in the Kindle menu.

## Usage Notes

- GTKKS on Kindle uses the e-ink display, which has slower refresh rates than regular screens.
- The application is optimized for the Kindle's screen size and e-ink display.
- Battery usage may be higher when using GTKKS compared to regular e-reader functions.

## Troubleshooting

- If GTKKS doesn't appear in the menu, check that the installation script ran correctly.
- If the application crashes, check the logs at `/var/log/messages` on your Kindle.
- For network-related issues, ensure your Kindle is connected to Wi-Fi.

## Uninstalling

To uninstall GTKKS from your Kindle:

```bash
rm -rf /mnt/us/gtkks-kindle
rm -rf /mnt/us/extensions/gtkks
```

Then restart your Kindle. 