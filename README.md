# Remarkable Todoist

A Todoist client for the reMarkable 2 e-ink tablet. View, manage, and complete your Todoist tasks directly on your reMarkable.

## Features

- View all Todoist tasks with name, due date, project, and priority
- Touch-friendly interface optimized for e-ink display
- Works on stock reMarkable firmware (no Toltec required)

## Current Status

**Phase 1 Complete** - The app displays tasks from the Todoist API.

## Requirements

- reMarkable 2 tablet
- Todoist account with API token
- WiFi connection (for syncing)

## Building

The app is built **on the reMarkable device itself** (cross-compilation from desktop is not currently working).

### On the Device

```bash
cd ~/Remarkable_Todoist
mkdir -p build && cd build
cmake ..
make
```

This produces `build/remarkable-todoist`.

## Configuration

Create the config file with your Todoist API token:

```bash
mkdir -p ~/.config/remarkable-todoist
echo "[General]
apiToken=YOUR_TODOIST_API_TOKEN" > ~/.config/remarkable-todoist/config.ini
chmod 600 ~/.config/remarkable-todoist/config.ini
```

Get your API token from: https://todoist.com/prefs/integrations

## Running

### Manual Launch (via SSH)

```bash
# Stop the default UI
systemctl stop xochitl

# Set up display (if needed)
export QT_QPA_PLATFORM=epaper
export QT_QUICK_BACKEND=epaper

# Run the app
./remarkable-todoist

# When done, restart the default UI
systemctl start xochitl
```

### Launcher Notebook (No SSH Required)

After initial setup, you can launch the app by opening a special notebook:

1. Install the launcher: `./launcher/install.sh`
2. Create a notebook named exactly: **"Launch Todoist"**
3. Open that notebook to launch the app
4. Exit the app to return to the normal reMarkable UI

See `launcher/README.md` for details.

### Oxide Launcher (Toltec Users)

If you have Toltec/Oxide installed:

1. Run `./oxide/install.sh`
2. The app appears in your Oxide launcher

See `oxide/README.md` for details.

## Project Structure

```
remarkable-todoist/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── controllers/          # App controller (QML bridge)
│   ├── models/               # Task and TaskModel
│   ├── network/              # Todoist API client
│   └── config/               # Settings management
├── qml/
│   ├── main.qml              # Main UI
│   └── TaskDelegate.qml      # Task list item
├── launcher/                 # Stock firmware launcher
└── oxide/                    # Oxide/Toltec integration
```

## Development Notes

### Why Build On-Device?

Cross-compilation requires matching Qt6 libraries for the reMarkable's ARM architecture. The official reMarkable toolchain and Toltec Docker images don't currently provide compatible Qt6 Quick/QML libraries. Building on-device sidesteps this issue.

### Qt6 Quick/QML

reMarkable firmware 3.x uses Qt6 with Quick/QML, not Qt5 Widgets. The UI is implemented in QML for compatibility.

### Display Environment

The app requires e-paper display plugins:
- `QT_QPA_PLATFORM=epaper`
- `QT_QUICK_BACKEND=epaper`

These are set automatically by the launcher scripts.

## License

MIT
