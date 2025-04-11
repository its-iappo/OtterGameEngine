project(OtterStudioLib LANGUAGES CXX)

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    Definitions/*.cpp
)

file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS
    Declarations/*.h
)

add_library(OtterStudioLib STATIC
    ${LIB_SOURCES}
    ${LIB_HEADERS}
)

target_include_directories(OtterStudioLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Declarations
)
