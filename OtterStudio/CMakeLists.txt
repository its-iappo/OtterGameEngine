# OtterStudio/CMakeLists.txt

add_executable(OtterStudio
    Source/main.cpp
)

set_target_properties(OtterStudio PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-fmodules-ts" COMPILER_SUPPORTS_MODULES_TS)
check_cxx_compiler_flag("/experimental:module" MSVC_SUPPORTS_MODULES)

if (COMPILER_SUPPORTS_MODULES_TS OR MSVC_SUPPORTS_MODULES)
    message(STATUS "Modules are supported. Enabling OTTER_USE_MODULES.")
    target_compile_definitions(OtterStudio PRIVATE OTTER_USE_MODULES=1)
else()
    message(WARNING "Modules not supported by the compiler. Falling back to headers.")
endif()

target_link_libraries(OtterStudio PRIVATE OtterEngine)
