# OtterStudio/CMakeLists.txt

add_executable(OtterStudio
    Source/main.cpp
)

set_target_properties(OtterStudio PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_link_libraries(OtterStudio PRIVATE OtterEngine)
