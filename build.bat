@echo off

echo [INFO] === Otter Game Engine Setup ===

set BUILD_DIR=%~dp0build

echo [INFO] Cleaning build directory...

if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
    echo [INFO] Build directory cleaned: %BUILD_DIR%
) else (
    echo [INFO] Build directory does not exist: %BUILD_DIR%
)

:: === Path for vcpkg (inside project folder)
set VCPKG_DIR=%~dp0vcpkg

if not exist "%VCPKG_DIR%" (
    echo [INFO] Cloning vcpkg...
    git clone https://github.com/microsoft/vcpkg.git "%VCPKG_DIR%"
)

echo [INFO] Bootstrapping vcpkg...
call "%VCPKG_DIR%\bootstrap-vcpkg.bat" -disableMetrics

:: === Set environment variables
set VCPKG_ROOT=%VCPKG_DIR%
set CMAKE_TOOLCHAIN_FILE=%VCPKG_DIR%\scripts\buildsystems\vcpkg.cmake

:: === Build and source dirs
set BUILD_DIR=%~dp0build
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)
set SOURCE_DIR=%~dp0
set SOURCE_DIR=%SOURCE_DIR:~0,-1%

:: === Debug info
echo [DEBUG] BUILD_DIR: %BUILD_DIR%
echo [DEBUG] VCPKG_ROOT: %VCPKG_ROOT%
echo [DEBUG] CMAKE_TOOLCHAIN_FILE: %CMAKE_TOOLCHAIN_FILE%
echo [DEBUG] SOURCE_DIR: %SOURCE_DIR%

:: === Run CMake configure and build
cmake -G "Visual Studio 17 2022" -A x64 -B "%BUILD_DIR%" -S "%SOURCE_DIR%" -DCMAKE_TOOLCHAIN_FILE="%CMAKE_TOOLCHAIN_FILE%"
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b 1
)

cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    echo [ERROR] CMake build failed.
    pause
    exit /b 1
)

echo [SUCCESS] Otter Game Engine compiled successfully!
pause