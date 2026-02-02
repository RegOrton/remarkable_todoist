#!/bin/bash
#
# Install the Todoist launcher notebook system
# Run this script ON the reMarkable device
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/.."

echo "Installing Todoist Launcher..."

# Check for inotifywait
if ! command -v inotifywait &> /dev/null; then
    echo "Error: inotifywait not found. Installing inotify-tools..."
    echo "If this fails, you may need to install it manually."
    # Try opkg if available, otherwise give instructions
    if command -v opkg &> /dev/null; then
        opkg install inotify-tools
    else
        echo "Please install inotify-tools manually or copy the binary to the device."
        exit 1
    fi
fi

# Copy the main app executable
if [ -f "$PROJECT_DIR/remarkable-todoist" ]; then
    echo "  Copying app to /opt/bin/..."
    sudo cp "$PROJECT_DIR/remarkable-todoist" /opt/bin/
    sudo chmod +x /opt/bin/remarkable-todoist
else
    echo "Warning: remarkable-todoist executable not found. Build it first with 'make'"
fi

# Copy the launcher script
echo "  Installing launcher script..."
sudo cp "$SCRIPT_DIR/todoist-launcher.sh" /opt/bin/
sudo chmod +x /opt/bin/todoist-launcher.sh

# Install the systemd service
echo "  Installing systemd service..."
sudo cp "$SCRIPT_DIR/todoist-launcher.service" /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable todoist-launcher.service
sudo systemctl start todoist-launcher.service

echo ""
echo "Done! Now create a notebook named exactly:"
echo ""
echo "    Launch Todoist"
echo ""
echo "Opening that notebook will launch the app."
echo ""
echo "Service status:"
systemctl status todoist-launcher.service --no-pager || true
