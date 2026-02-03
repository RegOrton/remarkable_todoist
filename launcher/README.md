# Launcher Notebook

Launch Remarkable Todoist by opening a special notebook - no SSH required after setup.

## How It Works

1. A background service watches for a notebook named "Launch Todoist" to be opened
2. When you open it, Xochitl stops and the Todoist app launches
3. When you exit the app (tap the Exit button), Xochitl restarts

## Installation

Prerequisites: The app binary must already be at `/opt/bin/remarkable-todoist` and
`inotifywait` must be at `/usr/local/bin/inotifywait`. See the main README for
cross-compilation and deployment instructions.

```bash
# Deploy launcher scripts (from host machine)
scp launcher/todoist-launcher.sh root@10.11.99.1:/opt/bin/
scp launcher/todoist-launcher.service root@10.11.99.1:/etc/systemd/system/
ssh root@10.11.99.1 "chmod +x /opt/bin/todoist-launcher.sh && \
    systemctl daemon-reload && systemctl enable --now todoist-launcher.service"
```

Then create a new notebook in Xochitl named exactly: **Launch Todoist**

## How the Watcher Works

- The service waits 15 seconds after xochitl boots to avoid false triggers from
  xochitl's startup file scanning
- It watches for `open` events on the notebook's `.content` file
- When triggered, it stops xochitl, sets up the e-paper display environment, and
  launches the app
- On app exit, xochitl is restarted and the watcher resumes after another 15s delay

## Uninstall

```bash
ssh root@10.11.99.1
systemctl stop todoist-launcher.service
systemctl disable todoist-launcher.service
rm /etc/systemd/system/todoist-launcher.service
rm /opt/bin/todoist-launcher.sh
rm /opt/bin/remarkable-todoist
systemctl daemon-reload
```

## Troubleshooting

Check if the service is running:
```bash
ssh root@10.11.99.1 "systemctl status todoist-launcher.service"
```

View logs:
```bash
ssh root@10.11.99.1 "journalctl -u todoist-launcher.service -f"
```

Make sure your notebook is named exactly "Launch Todoist" (case-sensitive).
