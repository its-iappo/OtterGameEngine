# OtterStudio/CMakeLists.txt

add_executable(OtterStudio
    Source/main.cpp
)

set_target_properties(OtterStudio PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    MACOSX_BUNDLE TRUE
)

target_link_libraries(OtterStudio PRIVATE OtterEngine)

set_target_icon(OtterStudio "OtterStudioIcon")

# --- Shader copy ---
set(SHADER_SOURCE_DIR ${CMAKE_BINARY_DIR}/OtterEngine/Shaders)
set(SHADER_DEST_DIR $<TARGET_FILE_DIR:OtterStudio>/Shaders)

add_custom_command(TARGET OtterStudio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHADER_SOURCE_DIR}"
        "${SHADER_DEST_DIR}"
    COMMENT "Copying compiled shaders to OtterStudio output directory"
)

add_dependencies(OtterStudio Shaders)