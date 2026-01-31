cmake_minimum_required(VERSION 3.16)
project(remarkable-todoist VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt5 components
find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

# Collect all source files
set(SOURCES
    src/main.cpp
    src/models/task.cpp
    src/models/taskmodel.cpp
    src/config/settings.cpp
    src/network/todoist_client.cpp
    src/views/taskdelegate.cpp
    src/views/tasklistview.cpp
)

# Create executable
add_executable(remarkable-todoist ${SOURCES})

# Link Qt libraries
target_link_libraries(remarkable-todoist
    Qt5::Widgets
    Qt5::Network
)

# Set output directory
set_target_properties(remarkable-todoist PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
