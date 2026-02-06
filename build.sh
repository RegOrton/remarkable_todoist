#!/bin/bash
#
# Master build script for Remarkable Todoist
#
# Usage:
#   ./build.sh desktop          # Build for desktop
#   ./build.sh desktop run      # Build and run on desktop
#   ./build.sh remarkable       # Cross-compile for reMarkable
#   ./build.sh remarkable deploy # Cross-compile and deploy to device
#   ./build.sh all              # Build both desktop and reMarkable
#   ./build.sh clean            # Clean all build directories
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo_info() {
    echo -e "${GREEN}==>${NC} $1"
}

echo_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE} $1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

show_help() {
    cat << EOF
Remarkable Todoist Build System

Usage: $0 <target> [command]

Targets:
  desktop       Build for local desktop (development/testing)
  remarkable    Cross-compile for reMarkable 2 device
  all           Build both desktop and reMarkable

Commands (target-specific):
  run           Build and run (desktop only)
  deploy        Build and deploy to device (remarkable only)
  clean         Clean and rebuild

Examples:
  $0 desktop              # Build for desktop
  $0 desktop run          # Build and run on desktop
  $0 remarkable           # Cross-compile for reMarkable
  $0 remarkable deploy    # Build and deploy to device
  $0 all                  # Build both versions
  $0 clean                # Clean all build directories

Environment Variables:
  REMARKABLE_IP          Device IP (default: 10.11.99.1)
EOF
}

# Clean all build directories
clean_all() {
    echo_header "Cleaning All Build Directories"

    if [ -d "$SCRIPT_DIR/build" ]; then
        echo_info "Removing build/"
        rm -rf "$SCRIPT_DIR/build"
    fi

    if [ -d "$SCRIPT_DIR/build-rm" ]; then
        echo_info "Removing build-rm/"
        rm -rf "$SCRIPT_DIR/build-rm"
    fi

    echo_info "Clean complete"
}

# Build desktop version
build_desktop() {
    echo_header "Building Desktop Version"
    "$SCRIPT_DIR/build-desktop.sh" "${1:-build}"
}

# Build reMarkable version
build_remarkable() {
    echo_header "Building reMarkable Version"
    "$SCRIPT_DIR/build-remarkable.sh" "${1:-build}"
}

# Main logic
case "${1:-}" in
    desktop)
        build_desktop "${2:-}"
        ;;
    remarkable|rm)
        build_remarkable "${2:-}"
        ;;
    all)
        build_desktop
        build_remarkable
        ;;
    clean)
        clean_all
        ;;
    help|--help|-h)
        show_help
        ;;
    "")
        echo "Error: No target specified"
        echo ""
        show_help
        exit 1
        ;;
    *)
        echo "Error: Unknown target '$1'"
        echo ""
        show_help
        exit 1
        ;;
esac
