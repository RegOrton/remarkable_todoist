# CMake toolchain file for reMarkable 2 cross-compilation
#
# Usage:
#   source ~/remarkable-toolchain/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi
#   cmake -B build-rm -DCMAKE_TOOLCHAIN_FILE=cmake/remarkable.cmake
#   cmake --build build-rm
#

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Use environment variables set by the toolchain's environment script
if(DEFINED ENV{CC})
    set(CMAKE_C_COMPILER $ENV{CC})
endif()

if(DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
endif()

if(DEFINED ENV{SDKTARGETSYSROOT})
    set(CMAKE_SYSROOT $ENV{SDKTARGETSYSROOT})
    set(CMAKE_FIND_ROOT_PATH $ENV{SDKTARGETSYSROOT})
endif()

# Search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ARM-specific flags for reMarkable 2 (Cortex-A7)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a -mfpu=neon -mfloat-abi=hard" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -mfpu=neon -mfloat-abi=hard" CACHE STRING "" FORCE)
