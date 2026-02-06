#!/bin/bash
#
# Pull device libraries from reMarkable for cross-compilation
# Creates sysroot at /tmp/rm-sysroot with proper symlinks
#
# Usage: ./scripts/pull-sysroot.sh [device_ip]
#

set -e

DEVICE_IP="${1:-${REMARKABLE_IP:-10.11.99.1}}"
DEVICE_USER="root"
SYSROOT="/tmp/rm-sysroot"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

echo_info() {
    echo -e "${GREEN}==>${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}Warning:${NC} $1"
}

echo_error() {
    echo -e "${RED}Error:${NC} $1"
}

echo_step() {
    echo -e "${BLUE}===${NC} $1"
}

# Check if device is reachable
check_device() {
    echo_step "Checking device connection"

    if ! ping -c 1 -W 2 "$DEVICE_IP" &> /dev/null; then
        echo_error "Cannot reach device at $DEVICE_IP"
        echo ""
        echo "Make sure your reMarkable is connected via:"
        echo "  - USB cable (default IP: 10.11.99.1)"
        echo "  - WiFi (check device settings for IP address)"
        echo ""
        echo "You can also set REMARKABLE_IP environment variable:"
        echo "  export REMARKABLE_IP=192.168.1.xxx"
        exit 1
    fi

    echo_info "Device reachable at $DEVICE_IP"
}

# Pull libraries from device
pull_libraries() {
    echo_step "Pulling libraries from device"

    mkdir -p "$SYSROOT"

    echo "Downloading libraries (this may take 1-2 minutes)..."
    if ! ssh "$DEVICE_USER@$DEVICE_IP" "tar czf - /usr/lib/*.so* /lib/*.so* 2>/dev/null" | tar xzf - -C "$SYSROOT/"; then
        echo_error "Failed to pull libraries from device"
        exit 1
    fi

    echo_info "Libraries downloaded successfully"
}

# Create symlinks for linker
create_symlinks() {
    echo_step "Creating linker symlinks"

    local count=0

    # Create .so symlinks for .so.X files in /usr/lib
    if [ -d "$SYSROOT/usr/lib" ]; then
        cd "$SYSROOT/usr/lib"
        for lib in *.so.*; do
            [ -e "$lib" ] || continue

            # Extract base name (e.g., libQt6Core.so from libQt6Core.so.6)
            base=$(echo "$lib" | sed 's/\.so\..*/\.so/')

            # Create symlink if it doesn't exist
            if [ ! -e "$base" ]; then
                ln -s "$lib" "$base"
                count=$((count + 1))
            fi
        done
    fi

    # Create .so symlinks for .so.X files in /lib
    if [ -d "$SYSROOT/lib" ]; then
        cd "$SYSROOT/lib"
        for lib in *.so.*; do
            [ -e "$lib" ] || continue

            base=$(echo "$lib" | sed 's/\.so\..*/\.so/')

            if [ ! -e "$base" ]; then
                ln -s "$lib" "$base"
                count=$((count + 1))
            fi
        done
    fi

    echo_info "Created $count symlinks for linker"
}

# Verify critical libraries
verify_libraries() {
    echo_step "Verifying libraries"

    local missing=()

    # Check for Qt6 libraries
    for lib in Qt6Core Qt6Gui Qt6Network Qt6Qml Qt6Quick; do
        if [ ! -e "$SYSROOT/usr/lib/lib${lib}.so" ] && [ ! -e "$SYSROOT/lib/lib${lib}.so" ]; then
            missing+=("lib${lib}")
        fi
    done

    # Check for ICU libraries
    for lib in icuuc icui18n icudata; do
        if [ ! -e "$SYSROOT/usr/lib/lib${lib}.so" ]; then
            missing+=("lib${lib}")
        fi
    done

    if [ ${#missing[@]} -gt 0 ]; then
        echo_warn "Some libraries are missing:"
        for lib in "${missing[@]}"; do
            echo "  - $lib"
        done
        echo ""
        echo "The build may fail. Make sure your reMarkable device has these libraries."
    else
        echo_info "All critical libraries present"
    fi

    # Check for optional OCR libraries
    if [ ! -e "$SYSROOT/usr/lib/libtesseract.so" ] && [ ! -e "$SYSROOT/lib/libtesseract.so" ]; then
        echo ""
        echo_warn "Tesseract OCR libraries not found on device"
        echo "The app will build without handwriting recognition support."
        echo "To add OCR support later, install Tesseract on the device."
    fi
}

# Show summary
show_summary() {
    echo ""
    echo_info "Sysroot setup complete!"
    echo ""
    echo "Location: $SYSROOT"
    echo "Size: $(du -sh $SYSROOT | cut -f1)"
    echo ""
    echo "You can now build for reMarkable:"
    echo "  ./build.sh remarkable"
    echo ""
}

# Main
main() {
    echo ""
    echo_info "Setting up reMarkable sysroot"
    echo ""

    check_device
    pull_libraries
    create_symlinks
    verify_libraries
    show_summary
}

main "$@"
