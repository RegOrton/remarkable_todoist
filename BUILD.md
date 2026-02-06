# Building Remarkable Todoist

This document describes how to build Remarkable Todoist for both desktop development and the reMarkable 2 device.

## Quick Start

```bash
# Desktop development (testing on your computer)
./build.sh desktop run

# Cross-compile for reMarkable device
./build.sh remarkable

# Deploy to device
./build.sh remarkable deploy
```

## Build Targets

### Desktop Build

Build and run on your local machine for development and testing.

```bash
# Build only
./build.sh desktop

# Build and run
./build.sh desktop run

# Clean and rebuild
./build.sh desktop clean
```

Or use the direct script:
```bash
./build-desktop.sh          # Build
./build-desktop.sh run      # Build and run
./build-desktop.sh clean    # Clean and rebuild
```

**Requirements:**
- CMake 3.16+
- Qt6 (Quick, QML, Network, Controls)
- Tesseract OCR (libtesseract-dev)
- Leptonica (libleptonica-dev)
- C++17 compiler

**Install dependencies (Debian/Ubuntu):**
```bash
sudo apt install cmake build-essential \
    qt6-base-dev qt6-declarative-dev \
    libtesseract-dev libleptonica-dev \
    tesseract-ocr-eng
```

### reMarkable Build

Cross-compile for the reMarkable 2 device (ARM32).

```bash
# Build only
./build.sh remarkable

# Build and deploy to device
./build.sh remarkable deploy

# Clean and rebuild
./build.sh remarkable clean
```

Or use the direct script:
```bash
./build-remarkable.sh          # Build
./build-remarkable.sh deploy   # Build and deploy
./build-remarkable.sh clean    # Clean and rebuild
```

**Requirements:**
- ARM cross-compiler (`arm-linux-gnueabihf-g++`)
- Qt6 development tools (moc, rcc)
- Device sysroot at `/tmp/rm-sysroot`

**Install cross-compiler (Debian/Ubuntu):**
```bash
sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

**Set up sysroot:**

The sysroot contains libraries from your reMarkable device needed for linking.

1. Connect your reMarkable via USB
2. Run the sysroot pull script (if available):
   ```bash
   ./scripts/pull-sysroot.sh
   ```

   Or manually:
   ```bash
   mkdir -p /tmp/rm-sysroot
   ssh root@10.11.99.1 "tar czf - /usr/lib/*.so* /lib/*.so*" | \
       tar xzf - -C /tmp/rm-sysroot/
   ```

### Build All

Build both desktop and reMarkable versions:

```bash
./build.sh all
```

### Clean All

Remove all build directories:

```bash
./build.sh clean
```

## Build Output

### Desktop Build
- Binary: `build/remarkable-todoist`
- Run directly: `./build/remarkable-todoist`

### reMarkable Build
- Binary: `build-rm/remarkable-todoist`
- Architecture: ARM 32-bit
- Deploy: `scp build-rm/remarkable-todoist root@10.11.99.1:/opt/bin/`

## Environment Variables

### REMARKABLE_IP

Set the IP address of your reMarkable device (default: `10.11.99.1`):

```bash
export REMARKABLE_IP=10.11.99.2
./build.sh remarkable deploy
```

Or inline:
```bash
REMARKABLE_IP=10.11.99.2 ./build.sh remarkable deploy
```

## Build System Structure

```
.
├── build.sh                    # Master build script
├── build-desktop.sh            # Desktop build script
├── build-remarkable.sh         # reMarkable cross-compile script
├── build-rm.sh                 # Legacy cross-compile script (simple)
├── CMakeLists.txt              # CMake configuration (desktop)
├── build/                      # Desktop build output
└── build-rm/                   # reMarkable build output
```

## Troubleshooting

### Desktop Build Issues

**Qt6 not found:**
```bash
sudo apt install qt6-base-dev qt6-declarative-dev
```

**Tesseract not found:**
```bash
sudo apt install libtesseract-dev libleptonica-dev tesseract-ocr-eng
```

**Missing language data:**
```bash
sudo apt install tesseract-ocr-eng
```

### reMarkable Build Issues

**Cross-compiler not found:**
```bash
sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

**Sysroot not found:**
- Ensure your reMarkable is connected
- Check IP address with `ping 10.11.99.1`
- Re-run sysroot setup (see above)

**Qt tools not found:**
- Qt6 development tools must be installed on the host
- Check paths in `build-remarkable.sh` (MOC, RCC variables)

**Linking errors:**
- Verify sysroot contains Qt6 libraries
- Check that device has Qt6 installed (reMarkable 3.x firmware)

### Deployment Issues

**Cannot connect to device:**
- Ensure device is connected via USB
- Check IP: `ping 10.11.99.1`
- Verify SSH access: `ssh root@10.11.99.1`

**Permission denied:**
```bash
ssh root@10.11.99.1 "chmod +x /opt/bin/remarkable-todoist"
```

## Development Workflow

1. **Make changes** to source code
2. **Test on desktop:**
   ```bash
   ./build.sh desktop run
   ```
3. **Build for reMarkable:**
   ```bash
   ./build.sh remarkable
   ```
4. **Deploy and test on device:**
   ```bash
   ./build.sh remarkable deploy
   ssh root@10.11.99.1 /opt/bin/remarkable-todoist
   ```

## CMake vs Manual Build

### Desktop: CMake (Recommended)

The desktop build uses CMake for better IDE integration and dependency management.

**Advantages:**
- Automatic dependency detection
- IDE support (Qt Creator, CLion, VS Code)
- Easier to maintain

### reMarkable: Manual Build

The reMarkable build uses a manual bash script for better control over cross-compilation.

**Advantages:**
- Full control over compiler flags
- Easier to debug cross-compilation issues
- No CMake toolchain file needed

## Further Reading

- [README.md](README.md) - Project overview and setup
- [.planning/ROADMAP.md](.planning/ROADMAP.md) - Project roadmap
- [launcher/README.md](launcher/README.md) - Device launcher setup
