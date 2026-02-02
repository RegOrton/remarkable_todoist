# Launcher Notebook

Launch Remarkable Todoist by opening a special notebook - no SSH required after setup.

## How It Works

1. A background service watches for a notebook named "Launch Todoist" to be opened
2. When you open it, Xochitl stops and the Todoist app launches
3. When you exit the app (tap the Exit button), Xochitl restarts

## Installation

1. Build the app on the device:
   ```bash
   cd ~/Remarkable_Todoist
   make
   ```

2. Run the install script:
   ```bash
   ./launcher/install.sh
   ```

3. Create a new notebook in Xochitl named exactly: **Launch Todoist**

4. Open that notebook to launch the app.

## Requirements

- `inotifywait` (from inotify-tools) must be available on the device
- If not installed, you may need to compile it for ARM or find a binary

## Manual Installation

```bash
# Copy files
sudo cp remarkable-todoist /opt/bin/
sudo cp launcher/todoist-launcher.sh /opt/bin/
sudo chmod +x /opt/bin/remarkable-todoist /opt/bin/todoist-launcher.sh

# Install service
sudo cp launcher/todoist-launcher.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable todoist-launcher.service
sudo systemctl start todoist-launcher.service
```

## Uninstall

```bash
sudo systemctl stop todoist-launcher.service
sudo systemctl disable todoist-launcher.service
sudo rm /etc/systemd/system/todoist-launcher.service
sudo rm /opt/bin/todoist-launcher.sh
sudo rm /opt/bin/remarkable-todoist
sudo systemctl daemon-reload
```

## Troubleshooting

Check if the service is running:
```bash
systemctl status todoist-launcher.service
```

View logs:
```bash
journalctl -u todoist-launcher.service -f
```

Make sure your notebook is named exactly "Launch Todoist" (case-sensitive).
