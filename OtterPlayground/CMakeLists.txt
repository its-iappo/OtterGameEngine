# OtterPlayground/CMakeLists.txt

add_executable(OtterPlayground
    Source/main.cpp
)

set_target_properties(OtterPlayground PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_link_libraries(OtterPlayground PRIVATE OtterEngine)
