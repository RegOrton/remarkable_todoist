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

### Prerequisites

On the host machine (arm64 Linux):
- `arm-linux-gnueabihf-g++` cross-compiler
- Qt6 development tools (`moc`, `rcc` from `qt6-base-dev-tools`)
- Device sysroot at `/tmp/rm-sysroot` (see step 1)

On the reMarkable:
- Stock firmware (tested on 3.24.0.149)
- USB connection to host (10.11.99.1)

### 1. Pull Sysroot (first time only)

Pull Qt6 and system libraries from the device for cross-compilation:
```bash
mkdir -p /tmp/rm-sysroot
ssh root@10.11.99.1 "tar czf - /usr/lib/*.so* /lib/*.so* 2>/dev/null" \
    | tar xzf - -C /tmp/rm-sysroot/
```

### 2. Build

Cross-compile on the host:
```bash
./build-rm.sh
```
This produces `build-rm/remarkable-todoist` (32-bit ARM).

### 3. Deploy

```bash
# Deploy binary
ssh root@10.11.99.1 "mkdir -p /opt/bin"
scp build-rm/remarkable-todoist root@10.11.99.1:/opt/bin/

# Deploy inotifywait (first time only - needed for launcher)
arm-linux-gnueabihf-gcc -static -O2 tools/inotifywait.c -o /tmp/inotifywait
scp /tmp/inotifywait root@10.11.99.1:/usr/local/bin/

# Deploy launcher scripts
scp launcher/todoist-launcher.sh root@10.11.99.1:/opt/bin/
scp launcher/todoist-launcher.service root@10.11.99.1:/etc/systemd/system/
ssh root@10.11.99.1 "chmod +x /opt/bin/todoist-launcher.sh /opt/bin/remarkable-todoist && \
    systemctl daemon-reload && systemctl enable --now todoist-launcher.service"
```

### 4. Configure API Token (first time only)

```bash
ssh root@10.11.99.1
mkdir -p ~/.config/remarkable-todoist
echo "[auth]
api_token=YOUR_TODOIST_API_TOKEN" > ~/.config/remarkable-todoist/config.conf
chmod 600 ~/.config/remarkable-todoist/config.conf
```

Get your API token from: https://todoist.com/prefs/integrations

### 5. Launch the App

1. Create a notebook named exactly: **"Launch Todoist"**
2. Open that notebook to launch the app
3. Tap **Exit** button to return to normal reMarkable UI

## Alternative Launch Methods

### Manual Launch (via SSH)

```bash
ssh root@10.11.99.1
systemctl stop xochitl
export HOME=/home/root
export QT_QPA_PLATFORM=epaper
export QT_QUICK_BACKEND=epaper
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="rotate=180:invertx"
/opt/bin/remarkable-todoist
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
├── build-rm.sh              # Cross-compilation script
├── tools/
│   └── inotifywait.c        # Minimal inotifywait for stock firmware
├── src/
│   ├── main.cpp              # Application entry point
│   ├── controllers/          # App controller (QML bridge)
│   ├── models/               # Task, TaskModel, SyncQueue
│   ├── network/              # Todoist API client, SyncManager
│   └── config/               # Settings management
├── qml/
│   ├── main.qml              # Main UI
│   └── TaskDelegate.qml      # Task list item
├── launcher/                 # Stock firmware launcher
└── oxide/                    # Oxide/Toltec integration
```

## Development Notes

### Cross-Compilation

The app is cross-compiled from an arm64 host using `build-rm.sh`. This script:
- Runs Qt6 MOC and RCC using host tools
- Compiles with `arm-linux-gnueabihf-g++` using host Qt6 headers
- Links against device libraries via sysroot at `/tmp/rm-sysroot`

The sysroot contains Qt6 `.so` files pulled from the device. This avoids needing a full cross-compilation SDK.

### Qt6 Quick/QML

reMarkable firmware 3.x uses Qt6 with Quick/QML, not Qt5 Widgets. The UI is implemented in QML for compatibility.

### Display Environment

The app requires e-paper display plugins and touch configuration:
- `QT_QPA_PLATFORM=epaper`
- `QT_QUICK_BACKEND=epaper`
- `QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="rotate=180:invertx"`
- `HOME=/home/root` (required when launched from systemd)

These are set automatically by the launcher scripts.

## License

MIT
