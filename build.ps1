# === Otter Game Engine Setup ===

$BUILD_DIR = "$PSScriptRoot\build"

Write-Host "[INFO] Cleaning build directory..."

if (Test-Path $BUILD_DIR) {
    Remove-Item -Recurse -Force $BUILD_DIR
    Write-Host "[INFO] Build directory cleaned: $BUILD_DIR"
} else {
    Write-Host "[INFO] Build directory does not exist: $BUILD_DIR"
}

# === Path for vcpkg (inside project folder)
$VCPKG_DIR = "$PSScriptRoot\vcpkg"

if (-Not (Test-Path $VCPKG_DIR)) {
    Write-Host "[INFO] Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git $VCPKG_DIR
}

Write-Host "[INFO] Bootstrapping vcpkg..."
& "$VCPKG_DIR\bootstrap-vcpkg.bat" -disableMetrics

# === Set environment variables
$env:VCPKG_ROOT = $VCPKG_DIR
$env:CMAKE_TOOLCHAIN_FILE = "$VCPKG_DIR\scripts\buildsystems\vcpkg.cmake"

# === Build and source dirs
if (-Not (Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Path $BUILD_DIR
}
$SOURCE_DIR = $PSScriptRoot

# === Debug info
Write-Host "[DEBUG] BUILD_DIR: $BUILD_DIR"
Write-Host "[DEBUG] VCPKG_ROOT: $env:VCPKG_ROOT"
Write-Host "[DEBUG] CMAKE_TOOLCHAIN_FILE: $env:CMAKE_TOOLCHAIN_FILE"
Write-Host "[DEBUG] SOURCE_DIR: $SOURCE_DIR"

# === Run CMake configure and build
cmake -G "Visual Studio 17 2022" -A x64 -B $BUILD_DIR -S $SOURCE_DIR -DCMAKE_TOOLCHAIN_FILE=$env:CMAKE_TOOLCHAIN_FILE
if ($?) {
    cmake --build $BUILD_DIR
} else {
    Write-Host "[ERROR] CMake configuration failed."
    Exit 1
}

if ($?) {
    Write-Host "[SUCCESS] Otter Game Engine compiled successfully!"
} else {
    Write-Host "[ERROR] CMake build failed."
    Exit 1
}
