cmake_minimum_required(VERSION 3.21)
project(OtterGameEngine VERSION 0.1.0 LANGUAGES CXX)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

# Aggiungi subdirectory
add_subdirectory(OtterEngine)
add_subdirectory(OtterPlayground)
add_subdirectory(OtterStudio)
