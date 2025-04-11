add_executable(OtterStudio)

target_sources(OtterStudio
    PRIVATE
        StudioMain.cpp
)

target_link_libraries(OtterStudio
    PRIVATE
        OtterStudioLib
)
