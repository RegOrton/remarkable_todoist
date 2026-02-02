#!/bin/bash
#
# Download and build inotify-tools for reMarkable
# Run this ON the device
#

set -e

VERSION="4.23.9.0"
TARBALL="inotify-tools-$VERSION.tar.gz"
URL="https://github.com/inotify-tools/inotify-tools/releases/download/$VERSION/$TARBALL"
BUILD_DIR="/tmp/inotify-tools-build"
INSTALL_DIR="/usr/local/bin"

echo "Building inotify-tools $VERSION..."

# Check if already installed
if command -v inotifywait &> /dev/null; then
    echo "inotifywait is already installed at: $(which inotifywait)"
    exit 0
fi

# Check for required tools
for tool in gcc make wget; do
    if ! command -v $tool &> /dev/null; then
        echo "Error: $tool is required but not found."
        exit 1
    fi
done

# Create build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Download source
echo "Downloading inotify-tools..."
wget -q --show-progress "$URL" -O "$TARBALL"

# Extract
echo "Extracting..."
tar xzf "$TARBALL"
cd "inotify-tools-$VERSION"

# Configure and build
echo "Configuring..."
./configure --prefix=/usr/local --disable-doxygen --quiet

echo "Building..."
make -j2 --quiet

# Install
echo "Installing..."
sudo make install --quiet

# Verify
if command -v inotifywait &> /dev/null; then
    echo ""
    echo "Success! inotifywait installed at: $(which inotifywait)"
    inotifywait --help 2>&1 | head -1
else
    # Try adding to PATH or copy manually
    if [ -f /usr/local/bin/inotifywait ]; then
        echo "Installed to /usr/local/bin/inotifywait"
        echo "You may need to add /usr/local/bin to your PATH"
    else
        echo "Error: Installation may have failed"
        exit 1
    fi
fi

# Cleanup
echo "Cleaning up..."
rm -rf "$BUILD_DIR"

echo "Done!"
