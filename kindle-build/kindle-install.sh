#!/bin/sh

# GTKKS Kindle Installation Script
# This script should be run on the Kindle device

echo "=== GTKKS Kindle Installation Script ==="

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
if [ -f "gtkks-kindle.tar.gz" ]; then
    tar -xzvf gtkks-kindle.tar.gz -C /mnt/us/
else
    echo "Error: gtkks-kindle.tar.gz not found."
    exit 1
fi

# Create desktop shortcut
echo "Creating desktop shortcut..."
cat > /mnt/us/extensions/gtkks/bin/gtkks.sh << 'EOF'
#!/bin/sh
cd /mnt/us/gtkks-kindle
./gtkks.sh
EOF

chmod +x /mnt/us/extensions/gtkks/bin/gtkks.sh

# Create menu item
echo "Creating menu item..."
cat > /mnt/us/extensions/gtkks/menu.json << 'EOF'
{
    "items": [
        {
            "name": "GTKKS",
            "priority": 1,
            "action": "/mnt/us/extensions/gtkks/bin/gtkks.sh"
        }
    ]
}
EOF

# Create icon
echo "Creating icon..."
if [ -f "/mnt/us/gtkks-kindle/resources/kindle/icon.svg" ]; then
    cp /mnt/us/gtkks-kindle/resources/kindle/icon.svg /mnt/us/extensions/gtkks/icons/gtkks.svg
fi

echo "Installation complete. Restart your Kindle to see GTKKS in the menu." 