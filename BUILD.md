# Building Remarkable Todoist

This document explains how to build the app for reMarkable 2.

## Quick Start

```bash
# Build for reMarkable (automatically sets up sysroot if needed)
./build.sh remarkable

# Deploy to device
./build.sh remarkable deploy
```

## Prerequisites

### On Build Machine (arm64 Linux)

1. **ARM Cross-Compiler**
   ```bash
   sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
   ```

2. **Qt6 Development Tools**
   ```bash
   sudo apt install qt6-base-dev-tools
   ```
   This provides `moc` and `rcc` tools needed for Qt compilation.

### On reMarkable Device

1. **Device Connection**
   - USB: Device appears at `10.11.99.1` (default)
   - WiFi: Check device settings for IP address

2. **SSH Access**
   - Password from Settings → Help → Copyrights and licenses → GPLv3 Compliance

## Build Process

### Automatic Setup (Recommended)

The build script automatically handles sysroot setup:

```bash
./build.sh remarkable
```

This will:
1. Check if sysroot exists at `/tmp/rm-sysroot`
2. If missing, automatically pull libraries from device
3. Create necessary linker symlinks
4. Detect available optional libraries (OCR)
5. Build with appropriate features enabled

### Manual Sysroot Setup

If you prefer manual setup or encounter issues:

```bash
./scripts/pull-sysroot.sh [device_ip]
```

This creates `/tmp/rm-sysroot` with:
- Device libraries from `/usr/lib` and `/lib`
- Proper `.so` symlinks for linking
- Verification of critical libraries

## Optional Features

### OCR / Handwriting Recognition

The app can be built with or without OCR support:

**Without OCR (default):**
- Build succeeds if Tesseract libraries are missing on device
- App displays clear error message when OCR features are accessed
- Phases 1 & 2 (task viewing, completion, sync) work perfectly

**With OCR:**
- Requires `libtesseract` and `liblept` on reMarkable device
- Enables Phase 3 (handwriting-based task creation)
- Automatically detected and enabled if libraries found in sysroot

To add OCR support:
1. Install Tesseract on device (via Toltec or custom build)
2. Re-pull sysroot: `./scripts/pull-sysroot.sh`
3. Rebuild: `./build.sh remarkable clean`

## Build Targets

### reMarkable (Production)
```bash
./build.sh remarkable          # Build only
./build.sh remarkable deploy   # Build and deploy
./build.sh remarkable clean    # Clean and rebuild
```

Produces: `build-rm/remarkable-todoist` (~254KB ARM32)

### Desktop (Testing)
```bash
./build.sh desktop             # Build only
./build.sh desktop run         # Build and run
./build.sh desktop clean       # Clean and rebuild
```

Produces: `build/remarkable-todoist` (native arch)

## Troubleshooting

### "Cannot reach device at 10.11.99.1"

**Solutions:**
1. Check USB connection
2. Verify device IP in Settings
3. Set custom IP: `export REMARKABLE_IP=192.168.x.x`

### "Cross-compiler not found"

**Solution:**
```bash
sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

### "Qt MOC not found"

**Solution:**
```bash
sudo apt install qt6-base-dev-tools
```

### "Linking failed: cannot find -lQt6Core"

This means linker symlinks are missing. The build script automatically creates them.

**Solution:**
```bash
./scripts/pull-sysroot.sh  # Re-pull and create symlinks
```

## Environment Variables

| Variable | Default | Purpose |
|----------|---------|---------|
| `REMARKABLE_IP` | `10.11.99.1` | Device IP address |

Example:
```bash
export REMARKABLE_IP=192.168.1.100
./build.sh remarkable deploy
```

## Technical Details

### Cross-Compilation Approach

We use a hybrid approach:
- **Host Qt tools** (`moc`, `rcc`) - version-independent
- **Host Qt headers** - architecture-independent
- **Device libraries** - actual runtime dependencies
- **Cross-compiler** - ARM32 code generation

This avoids needing a full cross-compilation SDK.

### Conditional Compilation

OCR support is controlled by `ENABLE_OCR` preprocessor flag:
- Defined automatically if Tesseract libraries found in sysroot
- Controls compilation of `handwriting_recognizer.cpp`
- Enables/disables OCR code paths at compile time
