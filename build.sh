#!/bin/bash

# === Otter Game Engine Setup ===

BUILD_DIR="$(pwd)/build"

echo "[INFO] Cleaning build directory..."

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
    echo "[INFO] Build directory cleaned: $BUILD_DIR"
else
    echo "[INFO] Build directory does not exist: $BUILD_DIR"
fi

# === Path for vcpkg (inside project folder)
VCPKG_DIR="$(pwd)/vcpkg"

if [ ! -d "$VCPKG_DIR" ]; then
    echo "[INFO] Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
fi

echo "[INFO] Bootstrapping vcpkg..."
"$VCPKG_DIR/bootstrap-vcpkg.sh" -disableMetrics

# === Set environment variables
export VCPKG_ROOT="$VCPKG_DIR"
export CMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake"

# === Build and source dirs
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi
SOURCE_DIR="$(pwd)"

# === Debug info
echo "[DEBUG] BUILD_DIR: $BUILD_DIR"
echo "[DEBUG] VCPKG_ROOT: $VCPKG_ROOT"
echo "[DEBUG] CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
echo "[DEBUG] SOURCE_DIR: $SOURCE_DIR"

# === Run CMake configure and build
cmake -G "Visual Studio 17 2022" -A x64 -B "$BUILD_DIR" -S "$SOURCE_DIR" -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE"
if [ $? -ne 0 ]; then
    echo "[ERROR] CMake configuration failed."
    exit 1
fi

cmake --build "$BUILD_DIR"
if [ $? -ne 0 ]; then
    echo "[ERROR] CMake build failed."
    exit 1
fi

echo "[SUCCESS] Otter Game Engine compiled successfully!"
