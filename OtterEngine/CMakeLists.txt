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
