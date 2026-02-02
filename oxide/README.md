# Oxide Launcher Installation

This folder contains files to add Remarkable Todoist to the Oxide launcher so you can launch it from the tablet UI without SSH.

## Files

- `remarkable-todoist.json` - Oxide launcher metadata
- `install.sh` - Installation script

## Installation

1. Build the app on the device:
   ```bash
   cd ~/Remarkable_Todoist
   make
   ```

2. Run the install script:
   ```bash
   ./oxide/install.sh
   ```

3. The app will appear in your Oxide launcher.

## Manual Installation

If the script doesn't work, run these commands manually:

```bash
# Copy executable
sudo cp remarkable-todoist /opt/bin/
sudo chmod +x /opt/bin/remarkable-todoist

# Create launcher entry
sudo mkdir -p /opt/etc/draft
sudo cp oxide/remarkable-todoist.json /opt/etc/draft/

# Restart Oxide
systemctl restart tarnish
```

## Uninstall

```bash
sudo rm /opt/bin/remarkable-todoist
sudo rm /opt/etc/draft/remarkable-todoist.json
systemctl restart tarnish
```
