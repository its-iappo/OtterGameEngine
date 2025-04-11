cmake_minimum_required(VERSION 3.21)

add_library(OtterEngineObjects OBJECT)

file(GLOB_RECURSE ENGINECORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Definitions/*.cpp
)

file(GLOB_RECURSE ENGINECORE_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Declarations/*.h
)

target_sources(OtterEngineObjects
    PRIVATE
        ${ENGINECORE_SOURCES}
        ${ENGINECORE_HEADERS}
)

target_include_directories(OtterEngineObjects
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Declarations
)

find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(OtterEngineObjects
    PUBLIC
        spdlog::spdlog
        glfw
        glm::glm
        Vulkan::Vulkan
)

add_library(OtterEngine STATIC)
target_link_libraries(OtterEngine PRIVATE OtterEngineObjects)
target_include_directories(OtterEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Declarations)

# Propaga le dipendenze manualmente su OtterEngine (non lo fa in automatico da OBJECT)
target_link_libraries(OtterEngine
    PUBLIC
        spdlog::spdlog
        glfw
        glm::glm
        Vulkan::Vulkan
)
