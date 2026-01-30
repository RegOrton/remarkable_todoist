#!/bin/bash
#
# Install development tools for Remarkable Todoist
# Run with: sudo ./scripts/install-dev-tools.sh
#

set -e

echo "=================================="
echo " Remarkable Todoist Dev Tools"
echo "=================================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run with sudo:"
    echo "  sudo $0"
    exit 1
fi

echo "Updating package lists..."
apt-get update

echo ""
echo "Installing build essentials..."
apt-get install -y \
    build-essential \
    cmake \
    pkg-config

echo ""
echo "Installing Qt 5 development packages..."
apt-get install -y \
    qtbase5-dev \
    qtbase5-dev-tools \
    libqt5widgets5 \
    libqt5network5 \
    libqt5gui5 \
    libqt5core5a

echo ""
echo "=================================="
echo " Installation Complete"
echo "=================================="
echo ""
echo "Verifying installation:"
echo ""

echo -n "CMake version: "
cmake --version | head -1

echo -n "Qt version: "
qmake --version | grep "Using Qt" || qmake -version | head -1

echo ""
echo "You can now build the project with:"
echo "  cd /home/reg/Remarkable_Todoist"
echo "  cmake -B build && cmake --build build"
echo ""
