cmake_minimum_required(VERSION 3.16)
project(remarkable-todoist VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 with Quick/QML for reMarkable 3.x
find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Qml Quick QuickControls2)
set(QT_VERSION_MAJOR 6)
message(STATUS "Using Qt6 Quick/QML")

# Find Tesseract OCR (for handwriting recognition)
# NOTE: For reMarkable cross-compilation, Tesseract .so files need to be in the sysroot.
# The build-rm.sh script may need PKG_CONFIG_PATH pointed at the sysroot's pkgconfig.
# For desktop development, install: libtesseract-dev libleptonica-dev tesseract-ocr-eng
find_package(PkgConfig REQUIRED)
pkg_check_modules(TESSERACT REQUIRED tesseract)
pkg_check_modules(LEPTONICA REQUIRED lept)

# Collect source files (no widget views)
set(SOURCES
    src/main.cpp
    src/models/task.cpp
    src/models/taskmodel.cpp
    src/models/sync_queue.cpp
    src/config/settings.cpp
    src/network/todoist_client.cpp
    src/network/sync_manager.cpp
    src/controllers/appcontroller.cpp
    src/ocr/handwriting_recognizer.cpp
)

# QML resources
qt_add_resources(QML_RESOURCES qml/qml.qrc)

# Create executable
add_executable(remarkable-todoist ${SOURCES} ${QML_RESOURCES})

# Include directories for Tesseract and Leptonica
target_include_directories(remarkable-todoist PRIVATE
    ${TESSERACT_INCLUDE_DIRS}
    ${LEPTONICA_INCLUDE_DIRS}
)

# Link Qt libraries and Tesseract
target_link_libraries(remarkable-todoist
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    ${TESSERACT_LIBRARIES}
    ${LEPTONICA_LIBRARIES}
)

# Set output directory
set_target_properties(remarkable-todoist PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
