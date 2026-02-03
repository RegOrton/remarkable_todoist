#!/bin/sh
#
# Install the Todoist launcher notebook system
# Run this script ON the reMarkable device after deploying binaries via scp
#
# Prerequisites (deploy from host first):
#   scp build-rm/remarkable-todoist root@10.11.99.1:/opt/bin/
#   scp /tmp/inotifywait root@10.11.99.1:/usr/local/bin/
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "Installing Todoist Launcher..."

# Check for inotifywait
if ! command -v inotifywait >/dev/null 2>&1 && ! [ -f /usr/local/bin/inotifywait ]; then
    echo "Error: inotifywait not found at /usr/local/bin/inotifywait"
    echo ""
    echo "Cross-compile and deploy it from the host machine:"
    echo "  arm-linux-gnueabihf-gcc -static -O2 tools/inotifywait.c -o /tmp/inotifywait"
    echo "  scp /tmp/inotifywait root@10.11.99.1:/usr/local/bin/"
    exit 1
fi

# Check for app binary
if ! [ -f /opt/bin/remarkable-todoist ]; then
    echo "Error: remarkable-todoist not found at /opt/bin/"
    echo ""
    echo "Cross-compile and deploy it from the host machine:"
    echo "  ./build-rm.sh"
    echo "  scp build-rm/remarkable-todoist root@10.11.99.1:/opt/bin/"
    exit 1
fi

# Copy the launcher script
echo "  Installing launcher script..."
cp "$SCRIPT_DIR/todoist-launcher.sh" /opt/bin/
chmod +x /opt/bin/todoist-launcher.sh

# Install the systemd service
echo "  Installing systemd service..."
cp "$SCRIPT_DIR/todoist-launcher.service" /etc/systemd/system/
systemctl daemon-reload
systemctl enable todoist-launcher.service
systemctl start todoist-launcher.service

echo ""
echo "Done! Now create a notebook named exactly:"
echo ""
echo "    Launch Todoist"
echo ""
echo "Opening that notebook will launch the app."
echo ""
systemctl status todoist-launcher.service --no-pager 2>/dev/null || true
