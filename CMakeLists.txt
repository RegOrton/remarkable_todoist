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

# Collect source files (no widget views)
set(SOURCES
    src/main.cpp
    src/models/task.cpp
    src/models/taskmodel.cpp
    src/models/sync_queue.cpp
    src/config/settings.cpp
    src/network/todoist_client.cpp
    src/controllers/appcontroller.cpp
)

# QML resources
qt_add_resources(QML_RESOURCES qml/qml.qrc)

# Create executable
add_executable(remarkable-todoist ${SOURCES} ${QML_RESOURCES})

# Link Qt libraries
target_link_libraries(remarkable-todoist
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

# Set output directory
set_target_properties(remarkable-todoist PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
