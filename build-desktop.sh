#!/bin/bash
#
# Build for desktop (local development/testing)
#
# Usage:
#   ./build-desktop.sh          # Build only
#   ./build-desktop.sh run      # Build and run
#   ./build-desktop.sh clean    # Clean and rebuild
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo_info() {
    echo -e "${GREEN}==>${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}Warning:${NC} $1"
}

echo_error() {
    echo -e "${RED}Error:${NC} $1"
}

# Check dependencies
check_dependencies() {
    echo_info "Checking dependencies..."

    local missing_deps=()

    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi

    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi

    if ! pkg-config --exists tesseract 2>/dev/null; then
        missing_deps+=("tesseract (libtesseract-dev)")
    fi

    if ! pkg-config --exists lept 2>/dev/null; then
        missing_deps+=("leptonica (libleptonica-dev)")
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        echo_error "Missing dependencies: ${missing_deps[*]}"
        echo "Install with: sudo apt install cmake build-essential libtesseract-dev libleptonica-dev qt6-base-dev qt6-declarative-dev"
        exit 1
    fi

    echo_info "All dependencies found"
}

# Clean build directory
clean_build() {
    echo_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
}

# Configure with CMake
configure() {
    echo_info "Configuring with CMake..."
    cd "$BUILD_DIR"
    cmake ..
}

# Build
build() {
    echo_info "Building..."
    cd "$BUILD_DIR"
    make -j$(nproc)

    if [ $? -eq 0 ]; then
        echo_info "Build successful!"
        echo_info "Binary: $BUILD_DIR/remarkable-todoist"
    else
        echo_error "Build failed!"
        exit 1
    fi
}

# Run the application
run() {
    echo_info "Running remarkable-todoist..."
    cd "$BUILD_DIR"
    ./remarkable-todoist
}

# Parse command line arguments
case "${1:-build}" in
    clean)
        check_dependencies
        clean_build
        configure
        build
        ;;
    run)
        check_dependencies
        if [ ! -d "$BUILD_DIR" ]; then
            configure
        fi
        build
        run
        ;;
    build|"")
        check_dependencies
        if [ ! -d "$BUILD_DIR" ]; then
            configure
        fi
        build
        ;;
    *)
        echo "Usage: $0 {build|run|clean}"
        echo ""
        echo "Commands:"
        echo "  build  - Build the application (default)"
        echo "  run    - Build and run the application"
        echo "  clean  - Clean build directory and rebuild"
        exit 1
        ;;
esac
