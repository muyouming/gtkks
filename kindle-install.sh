#!/bin/sh

# GTKKS Kindle Installation Script
# This script should be run on the Kindle device

# Create installation directory
mkdir -p /opt/amazon/gtkks

# Extract application files
tar -xzvf gtkks-kindle.tar.gz -C /mnt/us/

# Create desktop shortcut
cat > /mnt/us/extensions/gtkks/bin/gtkks.sh << 'EOF'
#!/bin/sh
cd /mnt/us/gtkks-kindle
./gtkks.sh
EOF

chmod +x /mnt/us/extensions/gtkks/bin/gtkks.sh

# Create menu item
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
mkdir -p /mnt/us/extensions/gtkks/icons
cp /mnt/us/gtkks-kindle/resources/icon.png /mnt/us/extensions/gtkks/icons/gtkks.png

echo "Installation complete. Restart your Kindle to see GTKKS in the menu." 