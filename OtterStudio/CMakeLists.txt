# OtterStudio/CMakeLists.txt

add_executable(OtterStudio
    Source/main.cpp
)

set_target_properties(OtterStudio PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_link_libraries(OtterStudio PRIVATE OtterEngine)

# --- Shader copy ---
# set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders)
# if(EXISTS ${CMAKE_SOURCE_DIR}/OtterEngine/Shaders)
#     add_custom_command(TARGET OtterStudio POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
#         COMMAND ${CMAKE_COMMAND} -E copy_directory
#             ${CMAKE_BINARY_DIR}/OtterEngine/Shaders
#             ${SHADER_BINARY_DIR}
#         COMMENT "Copying compiled shaders to OtterStudio output"
#     )
# endif()

# add_dependencies(OtterStudio Shaders)