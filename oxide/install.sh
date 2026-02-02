#!/bin/bash
#
# Install remarkable-todoist to Oxide launcher
# Run this script ON the reMarkable device
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/../remarkable-todoist"

echo "Installing Remarkable Todoist..."

# Check executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Executable not found at $EXECUTABLE"
    echo "Make sure you've built the app first (run 'make' in project root)"
    exit 1
fi

# Copy executable
echo "  Copying executable to /opt/bin/..."
sudo cp "$EXECUTABLE" /opt/bin/remarkable-todoist
sudo chmod +x /opt/bin/remarkable-todoist

# Create draft directory if needed
sudo mkdir -p /opt/etc/draft

# Copy launcher metadata
echo "  Installing Oxide launcher entry..."
sudo cp "$SCRIPT_DIR/remarkable-todoist.json" /opt/etc/draft/

# Restart Oxide
echo "  Restarting Oxide launcher..."
systemctl restart tarnish

echo ""
echo "Done! Todoist should now appear in your Oxide launcher."
