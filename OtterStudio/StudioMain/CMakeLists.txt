project(OtterStudio LANGUAGES CXX)

add_executable(OtterStudio
    StudioMain.cpp
)

target_link_libraries(OtterStudio
    PRIVATE OtterStudioLib
    PRIVATE OtterEngine
)
