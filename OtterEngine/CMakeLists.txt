# Define OtterEngine as a static library
add_library(OtterEngine STATIC)

include(CheckCXXCompilerFlag)

set(USE_CPP_MODULES OFF)
check_cxx_compiler_flag("-fmodules-ts" COMPILER_SUPPORTS_MODULES_TS)
check_cxx_compiler_flag("/experimental:module" MSVC_SUPPORTS_MODULES)

if (COMPILER_SUPPORTS_MODULES_TS OR MSVC_SUPPORTS_MODULES)
    set(USE_CPP_MODULES ON)
endif()

if(MSVC)
    target_compile_options(OtterEngine PRIVATE /wd5201)
    target_compile_options(OtterEngine PRIVATE /wd5244)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(OtterEngine PRIVATE -Wno-c5201)
    target_compile_options(OtterEngine PRIVATE -Wno-c5244)
endif()

# Source and header files (manual list for now, will use GLOB_RECURSE if needed)
if (USE_CPP_MODULES)
    target_compile_definitions(OtterEngine PUBLIC OTTER_USE_MODULES=1)
    message(STATUS "C++20 modules enabled")

    target_sources(OtterEngine PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Modules/Logger.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Modules/EngineCore.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineCore.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Headers/LoggerMacros.h
    )

    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/Modules/Logger.ixx PROPERTIES LANGUAGE CXX)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/Modules/EngineCore.ixx PROPERTIES LANGUAGE CXX)

    target_include_directories(OtterEngine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Headers
    )

else()
    message(WARNING "C++20 modules not supported by compiler. Falling back to headers.")

    target_sources(OtterEngine PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Headers/Logger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Headers/EngineCore.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/EngineCore.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Headers/LoggerMacros.h
    )

    target_include_directories(OtterEngine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Headers
    )
endif()

# Link required libraries
target_link_libraries(OtterEngine PUBLIC
    spdlog::spdlog
    glfw
    glm::glm
    Vulkan::Vulkan
)
