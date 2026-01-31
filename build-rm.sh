#!/bin/bash
#
# Cross-compile for reMarkable using device libraries
#

set -e

SYSROOT="/tmp/rm-sysroot"
CC="arm-linux-gnueabihf-gcc"
CXX="arm-linux-gnueabihf-g++"
OUTDIR="build-rm"

# Check for cross-compiler
if ! which $CXX > /dev/null 2>&1; then
    echo "Error: Cross-compiler $CXX not found"
    exit 1
fi

mkdir -p $OUTDIR

# Get Qt6 include paths from host (we'll adapt)
QT6_HOST_INC="/usr/include/aarch64-linux-gnu/qt6"

# MOC the files that need it
echo "Running MOC..."
/usr/lib/qt6/libexec/moc src/models/taskmodel.h -o $OUTDIR/moc_taskmodel.cpp
/usr/lib/qt6/libexec/moc src/controllers/appcontroller.h -o $OUTDIR/moc_appcontroller.cpp
/usr/lib/qt6/libexec/moc src/network/todoist_client.h -o $OUTDIR/moc_todoist_client.cpp

# Compile QML resources
echo "Compiling QML resources..."
/usr/lib/qt6/libexec/rcc qml/qml.qrc -o $OUTDIR/qrc_qml.cpp

# Compile sources
echo "Compiling..."
CXXFLAGS="-std=c++17 -fPIC -I$QT6_HOST_INC -I$QT6_HOST_INC/QtCore -I$QT6_HOST_INC/QtGui -I$QT6_HOST_INC/QtNetwork -I$QT6_HOST_INC/QtQml -I$QT6_HOST_INC/QtQuick -I$QT6_HOST_INC/QtQuickControls2"

SOURCES="
    src/main.cpp
    src/models/task.cpp
    src/models/taskmodel.cpp
    src/config/settings.cpp
    src/network/todoist_client.cpp
    src/controllers/appcontroller.cpp
    $OUTDIR/moc_taskmodel.cpp
    $OUTDIR/moc_appcontroller.cpp
    $OUTDIR/moc_todoist_client.cpp
    $OUTDIR/qrc_qml.cpp
"

for src in $SOURCES; do
    obj="$OUTDIR/$(basename ${src%.cpp}.o)"
    echo "  $src -> $obj"
    $CXX $CXXFLAGS -c "$src" -o "$obj"
done

# Link
echo "Linking..."
OBJS=$(ls $OUTDIR/*.o)
LDFLAGS="-L$SYSROOT/lib -Wl,-rpath,/lib"
LIBS="-lQt6Core -lQt6Gui -lQt6Network -lQt6Qml -lQt6Quick -lQt6QuickControls2"

$CXX $OBJS $LDFLAGS $LIBS -o $OUTDIR/remarkable-todoist

echo ""
echo "Build complete: $OUTDIR/remarkable-todoist"
file $OUTDIR/remarkable-todoist
