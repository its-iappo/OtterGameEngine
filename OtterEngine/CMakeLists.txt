# OtterEngine/CMakeLists.txt

# Define OtterEngine as a static library
add_library(OtterEngine STATIC)

# Source and header files
file(GLOB_RECURSE OTTERENGINE_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers/*.h
)

file(GLOB_RECURSE OTTERENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
)

target_sources(OtterEngine 
    PUBLIC ${OTTERENGINE_HEADERS}
    PRIVATE ${OTTERENGINE_SOURCES}
)

# Include path
target_include_directories(OtterEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers
)

# Link required libraries
target_link_libraries(OtterEngine PUBLIC
    spdlog::spdlog
    glfw
    glm::glm
    Vulkan::Vulkan
)

# Shader source dir
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders)

file(GLOB SHADER_SOURCES
    ${SHADER_SOURCE_DIR}/*.vert
    ${SHADER_SOURCE_DIR}/*.frag
)

# Ensure output directory exists
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

# Compile each shader with glslc
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV ${SHADER_BINARY_DIR}/${FILE_NAME}.spv)

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME}"
        VERBATIM
    )
    list(APPEND SPIRV_SHADERS ${SPIRV})
endforeach()

# Custom target so shaders are built together with engine
add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})

# Ensure engine build depends on shaders
add_dependencies(OtterEngine Shaders)