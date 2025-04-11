add_library(OtterStudioLib STATIC)

file(GLOB_RECURSE STUDIOLIB_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Declarations/*.h
)

file(GLOB_RECURSE STUDIOLIB_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Definitions/*.cpp
)

target_sources(OtterStudioLib
    PRIVATE
        ${STUDIOLIB_HEADERS}
        ${STUDIOLIB_SOURCES}
)

target_include_directories(OtterStudioLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Declarations
)

target_link_libraries(OtterStudioLib
    PUBLIC
        OtterEngine
)
