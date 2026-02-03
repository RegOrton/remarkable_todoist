#!/bin/bash
#
# Cross-compile for reMarkable 2 using device libraries
#

set -e

SYSROOT="/tmp/rm-sysroot"
CXX="arm-linux-gnueabihf-g++"
MOC="/usr/lib/qt6/libexec/moc"
RCC="/usr/lib/qt6/libexec/rcc"
OUTDIR="build-rm"

# Check for cross-compiler
if ! which $CXX > /dev/null 2>&1; then
    echo "Error: Cross-compiler $CXX not found"
    exit 1
fi

# Check sysroot
if [ ! -d "$SYSROOT/usr/lib" ]; then
    echo "Error: Sysroot not found at $SYSROOT"
    echo "Run: ./tools/pull-sysroot.sh"
    exit 1
fi

mkdir -p $OUTDIR

# Qt6 include paths (use host headers - they're arch-independent)
QT6_INC="/usr/include/aarch64-linux-gnu/qt6"

echo "=== MOC Processing ==="
$MOC src/models/taskmodel.h -o $OUTDIR/moc_taskmodel.cpp
$MOC src/models/sync_queue.h -o $OUTDIR/moc_sync_queue.cpp
$MOC src/controllers/appcontroller.h -o $OUTDIR/moc_appcontroller.cpp
$MOC src/network/todoist_client.h -o $OUTDIR/moc_todoist_client.cpp
$MOC src/network/sync_manager.h -o $OUTDIR/moc_sync_manager.cpp

echo "=== QML Resources ==="
$RCC qml/qml.qrc -o $OUTDIR/qrc_qml.cpp

echo "=== Compiling ==="
CXXFLAGS="-std=c++17 -fPIC -O2 \
    -I$QT6_INC \
    -I$QT6_INC/QtCore \
    -I$QT6_INC/QtGui \
    -I$QT6_INC/QtNetwork \
    -I$QT6_INC/QtQml \
    -I$QT6_INC/QtQuick \
    -I$QT6_INC/QtQuickControls2 \
    -Isrc"

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

for src in $SOURCES; do
    obj="$OUTDIR/$(basename ${src%.cpp}.o)"
    echo "  $src"
    $CXX $CXXFLAGS -c "$src" -o "$obj"
done

echo "=== Linking ==="
OBJS=$(ls $OUTDIR/*.o)
LDFLAGS="-L$SYSROOT/usr/lib -L$SYSROOT/lib \
    -Wl,-rpath-link,$SYSROOT/usr/lib:$SYSROOT/lib \
    -Wl,-rpath,/usr/lib:/lib \
    -Wl,--dynamic-linker=/lib/ld-linux-armhf.so.3 \
    -Wl,--allow-shlib-undefined"

LIBS="-lQt6Core -lQt6Gui -lQt6Network -lQt6Qml -lQt6Quick \
    -lQt6QuickControls2 -lQt6QuickTemplates2 \
    -lQt6DBus -lQt6QmlModels -lQt6QmlMeta -lQt6QmlWorkerScript \
    -licuuc -licui18n -licudata"

$CXX $OBJS $LDFLAGS $LIBS -o $OUTDIR/remarkable-todoist

echo ""
echo "=== Build Complete ==="
file $OUTDIR/remarkable-todoist
echo ""
echo "Deploy with: scp $OUTDIR/remarkable-todoist root@10.11.99.1:/opt/bin/"
