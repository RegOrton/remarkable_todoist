#!/bin/bash
#
# Set up reMarkable 2 cross-compilation toolchain
# Run with: ./scripts/setup-rm-toolchain.sh
#

set -e

TOOLCHAIN_DIR="$HOME/remarkable-toolchain"
TOOLCHAIN_VERSION="3.1.15"
TOOLCHAIN_URL="https://remarkable.engineering/oecore-x86_64-cortexa7hf-neon-rm11x-toolchain-${TOOLCHAIN_VERSION}.sh"

echo "=================================="
echo " reMarkable Toolchain Setup"
echo "=================================="
echo ""

# Check if already installed
if [ -d "$TOOLCHAIN_DIR" ] && [ -f "$TOOLCHAIN_DIR/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi" ]; then
    echo "Toolchain already installed at $TOOLCHAIN_DIR"
    echo ""
    echo "To use it, run:"
    echo "  source $TOOLCHAIN_DIR/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi"
    exit 0
fi

echo "Downloading reMarkable toolchain (this may take a few minutes)..."
cd /tmp

# Try to download the official toolchain
if ! wget -q --show-progress -O rm-toolchain.sh "$TOOLCHAIN_URL" 2>/dev/null; then
    echo ""
    echo "Official toolchain download failed. Trying Toltec toolchain..."

    # Fallback to Toltec's toolchain
    TOLTEC_URL="https://github.com/toltec-dev/toolchain/releases/download/v2.3/toolchain-v2.3-armv7l.tar.gz"

    if wget -q --show-progress -O toolchain.tar.gz "$TOLTEC_URL" 2>/dev/null; then
        echo "Extracting Toltec toolchain..."
        mkdir -p "$TOOLCHAIN_DIR"
        tar -xzf toolchain.tar.gz -C "$TOOLCHAIN_DIR"
        rm toolchain.tar.gz

        echo ""
        echo "Toltec toolchain installed to $TOOLCHAIN_DIR"
        echo ""
        echo "Note: Toltec toolchain uses different paths. Check the README for usage."
        exit 0
    fi

    echo ""
    echo "ERROR: Could not download toolchain."
    echo ""
    echo "Manual installation options:"
    echo "1. Download from https://remarkable.engineering/deploy/sdk/"
    echo "2. Use Docker: docker pull ghcr.io/toltec-dev/toolchain:latest"
    exit 1
fi

echo ""
echo "Installing toolchain to $TOOLCHAIN_DIR..."
chmod +x rm-toolchain.sh
./rm-toolchain.sh -d "$TOOLCHAIN_DIR" -y
rm rm-toolchain.sh

echo ""
echo "=================================="
echo " Installation Complete"
echo "=================================="
echo ""
echo "To use the toolchain, run:"
echo "  source $TOOLCHAIN_DIR/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi"
echo ""
echo "Then build with:"
echo "  cmake -B build-rm -DCMAKE_TOOLCHAIN_FILE=cmake/remarkable.cmake"
echo "  cmake --build build-rm"
echo ""
