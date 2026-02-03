#!/bin/sh
#
# Watches for the "Launch Todoist" notebook to be opened
# When detected, stops Xochitl and launches the Todoist app
#

# Add /usr/local/bin to PATH for inotifywait
export PATH="/usr/local/bin:$PATH"

NOTEBOOK_NAME="Launch Todoist"
APP_PATH="/opt/bin/remarkable-todoist"
METADATA_DIR="/home/root/.local/share/remarkable/xochitl"

# Find the notebook UUID by searching metadata files
find_notebook_uuid() {
    grep -l "\"visibleName\": \"$NOTEBOOK_NAME\"" "$METADATA_DIR"/*.metadata 2>/dev/null | head -n 1 | while read f; do basename "$f" .metadata; done
}

# Wait for the launcher notebook to appear (may not exist at boot)
wait_for_notebook() {
    while true; do
        uuid=$(find_notebook_uuid)
        if [ -n "$uuid" ]; then
            echo "$uuid"
            return
        fi
        echo "Waiting for '$NOTEBOOK_NAME' notebook to be created..."
        sleep 10
    done
}

# Watch Xochitl's log for notebook open events
watch_for_launch() {
    uuid=$(wait_for_notebook)

    echo "Watching for notebook: $NOTEBOOK_NAME (UUID: $uuid)"

    # Watch the notebook's .content file for access
    # When opened, Xochitl writes to this file
    while true; do
        inotifywait -q -e open,access "$METADATA_DIR/$uuid.content" 2>/dev/null

        echo "Launcher notebook opened! Starting Todoist..."

        # Stop Xochitl
        systemctl stop xochitl

        # Wait a moment for display to be free
        sleep 1

        # Launch Todoist app with e-paper display and touch settings
        export HOME=/home/root
        export QT_QPA_PLATFORM=epaper
        export QT_QUICK_BACKEND=epaper
        export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="rotate=180:invertx"
        $APP_PATH

        # App exited, restart Xochitl
        echo "App closed, restarting Xochitl..."
        systemctl start xochitl

        # Wait for Xochitl to fully start before watching again
        sleep 3
    done
}

watch_for_launch
