#!/bin/bash
#
# Cross-compile for reMarkable 2 using device libraries
#
# Prerequisites:
#   - ARM cross-compiler: arm-linux-gnueabihf-g++
#   - Device sysroot at /tmp/rm-sysroot (run: ./scripts/pull-sysroot.sh)
#   - Qt6 development tools (moc, rcc)
#
# Usage:
#   ./build-remarkable.sh           # Build only
#   ./build-remarkable.sh deploy    # Build and deploy to device
#   ./build-remarkable.sh clean     # Clean and rebuild
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SYSROOT="/tmp/rm-sysroot"
CXX="arm-linux-gnueabihf-g++"
MOC="/usr/lib/qt6/libexec/moc"
RCC="/usr/lib/qt6/libexec/rcc"
OUTDIR="$SCRIPT_DIR/build-rm"
DEVICE_IP="${REMARKABLE_IP:-10.11.99.1}"
DEVICE_USER="root"
DEPLOY_PATH="/opt/bin/remarkable-todoist"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
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

echo_step() {
    echo -e "${BLUE}===${NC} $1"
}

# Check for cross-compiler
check_cross_compiler() {
    if ! command -v $CXX &> /dev/null; then
        echo_error "Cross-compiler $CXX not found"
        echo "Install with: sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf"
        exit 1
    fi
    echo_info "Found cross-compiler: $(which $CXX)"
}

# Check for Qt tools
check_qt_tools() {
    if [ ! -f "$MOC" ]; then
        echo_error "Qt MOC not found at $MOC"
        echo "Install Qt6 development tools"
        exit 1
    fi

    if [ ! -f "$RCC" ]; then
        echo_error "Qt RCC not found at $RCC"
        echo "Install Qt6 development tools"
        exit 1
    fi
    echo_info "Found Qt tools: moc, rcc"
}

# Check for OCR libraries
check_ocr_libraries() {
    OCR_AVAILABLE=false

    if [ -e "$SYSROOT/usr/lib/libtesseract.so" ] || [ -e "$SYSROOT/lib/libtesseract.so" ]; then
        if [ -e "$SYSROOT/usr/lib/liblept.so" ] || [ -e "$SYSROOT/lib/liblept.so" ]; then
            OCR_AVAILABLE=true
            echo_info "OCR libraries found (Tesseract + Leptonica)"
        fi
    fi

    if [ "$OCR_AVAILABLE" = false ]; then
        echo_warn "OCR libraries not found - building without handwriting recognition"
        echo "To add OCR support: install tesseract and leptonica on device, then rebuild"
    fi
}

# Check sysroot
check_sysroot() {
    if [ ! -d "$SYSROOT/usr/lib" ]; then
        echo_warn "Sysroot not found at $SYSROOT"
        echo ""
        echo "Attempting to set up sysroot automatically..."
        echo ""

        # Check if pull-sysroot script exists
        if [ -f "$SCRIPT_DIR/scripts/pull-sysroot.sh" ]; then
            "$SCRIPT_DIR/scripts/pull-sysroot.sh" "$DEVICE_IP"
        else
            echo_error "Cannot find scripts/pull-sysroot.sh"
            echo ""
            echo "Manual setup:"
            echo "  1. Connect to reMarkable via USB"
            echo "  2. Run: mkdir -p $SYSROOT"
            echo "  3. Run: ssh root@$DEVICE_IP \"tar czf - /usr/lib/*.so* /lib/*.so*\" | tar xzf - -C $SYSROOT/"
            exit 1
        fi

        # Verify sysroot was created
        if [ ! -d "$SYSROOT/usr/lib" ]; then
            echo_error "Sysroot setup failed"
            exit 1
        fi
    fi
    echo_info "Found sysroot at $SYSROOT"

    # Verify critical symlinks exist
    if [ ! -e "$SYSROOT/usr/lib/libQt6Core.so" ] && [ ! -e "$SYSROOT/lib/libQt6Core.so" ]; then
        echo_warn "Linker symlinks missing, recreating..."
        create_linker_symlinks
    fi
}

# Create linker symlinks
create_linker_symlinks() {
    local created=0

    # Create .so symlinks in /usr/lib
    if [ -d "$SYSROOT/usr/lib" ]; then
        cd "$SYSROOT/usr/lib"
        for lib in *.so.*; do
            [ -e "$lib" ] || continue
            base=$(echo "$lib" | sed 's/\.so\..*/\.so/')
            if [ ! -e "$base" ]; then
                ln -s "$lib" "$base" 2>/dev/null || true
                created=$((created + 1))
            fi
        done
    fi

    # Create .so symlinks in /lib
    if [ -d "$SYSROOT/lib" ]; then
        cd "$SYSROOT/lib"
        for lib in *.so.*; do
            [ -e "$lib" ] || continue
            base=$(echo "$lib" | sed 's/\.so\..*/\.so/')
            if [ ! -e "$base" ]; then
                ln -s "$lib" "$base" 2>/dev/null || true
                created=$((created + 1))
            fi
        done
    fi

    if [ $created -gt 0 ]; then
        echo_info "Created $created linker symlinks"
    fi
}

# Clean build directory
clean_build() {
    echo_step "Cleaning build directory..."
    rm -rf "$OUTDIR"
    mkdir -p "$OUTDIR"
}

# Qt6 include paths (use host headers - they're arch-independent)
QT6_INC="/usr/include/aarch64-linux-gnu/qt6"

# Run Qt MOC preprocessor
run_moc() {
    echo_step "MOC Processing"
    $MOC src/models/taskmodel.h -o $OUTDIR/moc_taskmodel.cpp
    $MOC src/models/sync_queue.h -o $OUTDIR/moc_sync_queue.cpp
    $MOC src/controllers/appcontroller.h -o $OUTDIR/moc_appcontroller.cpp
    $MOC src/network/todoist_client.h -o $OUTDIR/moc_todoist_client.cpp
    $MOC src/network/sync_manager.h -o $OUTDIR/moc_sync_manager.cpp
    echo_info "Generated 5 MOC files"
}

# Compile QML resources
compile_qml() {
    echo_step "QML Resources"
    $RCC qml/qml.qrc -o $OUTDIR/qrc_qml.cpp
    echo_info "Compiled QML resources"
}

# Compile sources
compile_sources() {
    echo_step "Compiling"

    CXXFLAGS="-std=c++17 -fPIC -O2 \
        -I$QT6_INC \
        -I$QT6_INC/QtCore \
        -I$QT6_INC/QtGui \
        -I$QT6_INC/QtNetwork \
        -I$QT6_INC/QtQml \
        -I$QT6_INC/QtQuick \
        -I$QT6_INC/QtQuickControls2 \
        -I$SYSROOT/usr/include \
        -Isrc"

    # Add OCR flag if libraries are available
    if [ "$OCR_AVAILABLE" = true ]; then
        CXXFLAGS="$CXXFLAGS -DENABLE_OCR"
    fi

    # Base sources (always compiled)
    SOURCES="
        src/main.cpp
        src/models/task.cpp
        src/models/taskmodel.cpp
        src/models/sync_queue.cpp
        src/config/settings.cpp
        src/network/todoist_client.cpp
        src/network/sync_manager.cpp
        src/controllers/appcontroller.cpp
        $OUTDIR/moc_taskmodel.cpp
        $OUTDIR/moc_sync_queue.cpp
        $OUTDIR/moc_appcontroller.cpp
        $OUTDIR/moc_todoist_client.cpp
        $OUTDIR/moc_sync_manager.cpp
        $OUTDIR/qrc_qml.cpp
    "

    # Add OCR sources if libraries are available
    if [ "$OCR_AVAILABLE" = true ]; then
        SOURCES="$SOURCES src/ocr/handwriting_recognizer.cpp"
        echo_info "Including OCR support in build"
    fi

    local count=0
    for src in $SOURCES; do
        obj="$OUTDIR/$(basename ${src%.cpp}.o)"
        echo "  [$(($count + 1))] $src"
        $CXX $CXXFLAGS -c "$src" -o "$obj"
        count=$((count + 1))
    done

    echo_info "Compiled $count source files"
}

# Link binary
link_binary() {
    echo_step "Linking"

    OBJS=$(ls $OUTDIR/*.o)
    LDFLAGS="-L$SYSROOT/usr/lib -L$SYSROOT/lib \
        -Wl,-rpath-link,$SYSROOT/usr/lib:$SYSROOT/lib \
        -Wl,-rpath,/usr/lib:/lib \
        -Wl,--dynamic-linker=/lib/ld-linux-armhf.so.3 \
        -Wl,--allow-shlib-undefined"

    # Base libraries (always required)
    LIBS="-lQt6Core -lQt6Gui -lQt6Network -lQt6Qml -lQt6Quick \
        -lQt6QuickControls2 -lQt6QuickTemplates2 \
        -lQt6DBus -lQt6QmlModels -lQt6QmlMeta -lQt6QmlWorkerScript \
        -licuuc -licui18n -licudata"

    # Add OCR libraries if available
    if [ "$OCR_AVAILABLE" = true ]; then
        LIBS="$LIBS -ltesseract -llept"
    fi

    $CXX $OBJS $LDFLAGS $LIBS -o $OUTDIR/remarkable-todoist

    echo ""
    echo_info "Build Complete!"
    file $OUTDIR/remarkable-todoist
    ls -lh $OUTDIR/remarkable-todoist
}

# Deploy to device
deploy() {
    echo_step "Deploying to reMarkable"

    # Check if device is reachable
    if ! ping -c 1 -W 2 $DEVICE_IP &> /dev/null; then
        echo_error "Cannot reach device at $DEVICE_IP"
        echo "Make sure the device is connected via USB or WiFi"
        exit 1
    fi

    echo_info "Copying binary to device..."
    scp $OUTDIR/remarkable-todoist $DEVICE_USER@$DEVICE_IP:$DEPLOY_PATH

    echo_info "Setting permissions..."
    ssh $DEVICE_USER@$DEVICE_IP "chmod +x $DEPLOY_PATH"

    echo ""
    echo_info "Deployment complete!"
    echo "Run on device: ssh root@$DEVICE_IP '$DEPLOY_PATH'"
}

# Main build function
build() {
    cd "$SCRIPT_DIR"

    echo_info "Building for reMarkable 2 (ARM32)"
    echo ""

    check_cross_compiler
    check_qt_tools
    check_sysroot
    check_ocr_libraries

    mkdir -p "$OUTDIR"

    run_moc
    compile_qml
    compile_sources
    link_binary

    echo ""
    echo_info "Binary ready: $OUTDIR/remarkable-todoist"
    echo_info "Deploy with: $0 deploy"
}

# Parse command line arguments
case "${1:-build}" in
    clean)
        clean_build
        build
        ;;
    deploy)
        if [ ! -f "$OUTDIR/remarkable-todoist" ]; then
            build
        fi
        deploy
        ;;
    build|"")
        build
        ;;
    *)
        echo "Usage: $0 {build|deploy|clean}"
        echo ""
        echo "Commands:"
        echo "  build   - Cross-compile for reMarkable 2 (default)"
        echo "  deploy  - Build and deploy to device"
        echo "  clean   - Clean build directory and rebuild"
        echo ""
        echo "Environment variables:"
        echo "  REMARKABLE_IP - Device IP address (default: 10.11.99.1)"
        exit 1
        ;;
esac
