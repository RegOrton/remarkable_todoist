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
    echo "Error: inotifywait not found."
    echo ""
    if command -v opkg &> /dev/null; then
        echo "Installing via opkg..."
        opkg install inotify-tools
    else
        echo "The launcher notebook feature requires inotify-tools."
        echo "On stock firmware, you have two options:"
        echo ""
        echo "  1. Use manual SSH launch instead (see README.md)"
        echo "  2. Copy a static inotifywait binary to /usr/bin/"
        echo ""
        echo "You can get a static ARM binary from:"
        echo "  https://github.com/inotify-tools/inotify-tools/releases"
        echo ""
        exit 1
    fi
fi

# Copy the main app executable (from build directory)
if [ -f "$PROJECT_DIR/build/remarkable-todoist" ]; then
    echo "  Copying app to /opt/bin/..."
    sudo cp "$PROJECT_DIR/build/remarkable-todoist" /opt/bin/
    sudo chmod +x /opt/bin/remarkable-todoist
else
    echo "Error: remarkable-todoist executable not found."
    echo "Build it first with: mkdir -p build && cd build && cmake .. && make"
    exit 1
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
