# Remarkable Todoist

A Todoist client for the reMarkable 2 e-ink tablet. View, manage, and complete your Todoist tasks directly on your reMarkable.

## Features

- View all Todoist tasks with name, due date, project, and priority
- Mark tasks complete with checkbox (syncs to Todoist)
- Offline support - complete tasks without WiFi, syncs when reconnected
- Touch-friendly interface optimized for e-ink display
- Works on stock reMarkable firmware (no Toltec required)

## Current Status

**Phase 2 Complete** - View tasks and mark them complete with offline sync.

## Quick Start (Full Deployment)

### 1. Copy to Device

From your computer:
```bash
scp -r ~/Remarkable_Todoist root@10.11.99.1:~/
```

### 2. Build and Install

SSH to device and run:
```bash
ssh root@10.11.99.1

cd ~/Remarkable_Todoist

# Build the app
mkdir -p build && cd build
cmake .. && make
cd ..

# Configure API token (get from https://todoist.com/prefs/integrations)
mkdir -p ~/.config/remarkable-todoist
echo "[auth]
api_token=YOUR_TODOIST_API_TOKEN" > ~/.config/remarkable-todoist/config.conf
chmod 600 ~/.config/remarkable-todoist/config.conf

# Install launcher (automatically builds inotify-tools if needed)
./launcher/install.sh
```

### 3. Launch the App

1. Create a notebook named exactly: **"Launch Todoist"**
2. Open that notebook to launch the app
3. Tap **Exit** button to return to normal reMarkable UI

## Requirements

- reMarkable 2 tablet
- Todoist account with API token
- WiFi connection (for syncing)
- On device: `gcc`, `make`, `wget`, `cmake` (for building)

## Alternative Launch Methods

### Manual Launch (via SSH)

```bash
systemctl stop xochitl
export QT_QPA_PLATFORM=epaper
export QT_QUICK_BACKEND=epaper
./build/remarkable-todoist
systemctl start xochitl
```

### Oxide Launcher (Toltec Users)

If you have Toltec/Oxide installed:
```bash
./oxide/install.sh
```
The app appears in your Oxide launcher. See `oxide/README.md` for details.

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
