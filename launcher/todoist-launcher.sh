#!/bin/bash
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
    grep -l "\"visibleName\": \"$NOTEBOOK_NAME\"" "$METADATA_DIR"/*.metadata 2>/dev/null | head -1 | xargs -I{} basename {} .metadata
}

# Watch Xochitl's log for notebook open events
watch_for_launch() {
    local uuid=$(find_notebook_uuid)

    if [ -z "$uuid" ]; then
        echo "Launcher notebook '$NOTEBOOK_NAME' not found."
        echo "Create a notebook with this exact name: $NOTEBOOK_NAME"
        exit 1
    fi

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

        # Launch Todoist app with e-paper display settings
        export QT_QPA_PLATFORM=epaper
        export QT_QUICK_BACKEND=epaper
        $APP_PATH

        # App exited, restart Xochitl
        echo "App closed, restarting Xochitl..."
        systemctl start xochitl

        # Wait for Xochitl to fully start before watching again
        sleep 3
    done
}

watch_for_launch
