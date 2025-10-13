# OtterEngine/CMakeLists.txt

# Define OtterEngine as a static library
add_library(OtterEngine STATIC)

# Source and header files
file(GLOB_RECURSE OTTERENGINE_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers/*.hpp
)

file(GLOB_RECURSE OTTERENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
)

target_sources(OtterEngine 
    PUBLIC ${OTTERENGINE_HEADERS}
    PRIVATE ${OTTERENGINE_SOURCES}
)

# Include path
target_include_directories(OtterEngine 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Headers
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty
)

# Precompile headers
target_precompile_headers(OtterEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Headers/OtterPCH.h")

# Link required libraries
target_link_libraries(OtterEngine PUBLIC
    spdlog::spdlog
    glfw
    glm::glm
    Vulkan::Vulkan
    unofficial::shaderc::shaderc
)

# -------------------------
# Shader compilation
# -------------------------

find_program(GLSLC_EXECUTABLE NAMES glslc HINTS ${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/tools/shaderc)

if (NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found! Make sure shaderc is installed with vcpkg.")
else()
    message(STATUS "Found glslc: ${GLSLC_EXECUTABLE}")
endif()

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders)

file(GLOB_RECURSE SHADER_SOURCES
    ${SHADER_SOURCE_DIR}/*.vert
    ${SHADER_SOURCE_DIR}/*.frag
)

set(SPV_OUTPUTS "")
foreach(SHADER ${SHADER_SOURCES})
    file(RELATIVE_PATH REL_PATH ${SHADER_SOURCE_DIR} ${SHADER})
    set(SPV "${SHADER_BINARY_DIR}/${REL_PATH}.spv")
    get_filename_component(SPV_DIR ${SPV} DIRECTORY)
    file(MAKE_DIRECTORY ${SPV_DIR})

    add_custom_command(
        OUTPUT ${SPV}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${REL_PATH} -> ${SPV}"
        VERBATIM
    )
    list(APPEND SPV_OUTPUTS ${SPV})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPV_OUTPUTS})
