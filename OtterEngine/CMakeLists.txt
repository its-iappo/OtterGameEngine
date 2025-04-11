project(OtterEngine LANGUAGES CXX)

file(GLOB_RECURSE ENGINECORE_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/Definitions/*.cpp
)

# Aggiungi tutti i file .h da Declarations
file(GLOB_RECURSE ENGINECORE_HEADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/Declarations/*.h
)

# Aggiungi il target per il motore
add_library(${PROJECT_NAME} STATIC
    ${ENGINECORE_SOURCES}
    ${ENGINECORE_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Declarations
)

# Trova e collega le librerie
find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        spdlog::spdlog
        glfw
        glm::glm
        Vulkan::Vulkan
)