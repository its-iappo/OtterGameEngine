# OtterGameEngine/CMakeLists.txt (root file)

cmake_minimum_required(VERSION 3.27)

project(OtterGameEngine LANGUAGES CXX)

set(CMAKE_SUPPRESS_REGENERATION true)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

message(STATUS "CMake version found is: ${CMAKE_VERSION}")
message(STATUS "Using C++ standard: ${CMAKE_CXX_STANDARD}")

# Add vcpkg dependencies
find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)

# Set icon of executables function
function(set_target_icon target icon_base_name)
	message(STATUS "Setting icon for target: ${target}")
    message(STATUS "Icon base name: ${icon_base_name}")

	if(WIN32)
		set(icon_file "${CMAKE_CURRENT_SOURCE_DIR}/InternalResources/Icons/${icon_base_name}.ico")
		if(EXISTS ${icon_file})
			set(rc_file "${CMAKE_CURRENT_BINARY_DIR}/${target}_icon.rc")
			file(WRITE ${rc_file} "IDI_ICON1\tICON\tDISCARDABLE\t\"${icon_file}\"")

			target_sources(${target} PRIVATE ${rc_file})
            message(STATUS "Added RC file: ${rc_file}")

		else()
			message(WARNING "Icon file ${icon_file} not found!")
		endif()

	elseif(APPLE)
		set(icon_file "${CMAKE_CURRENT_SOURCE_DIR}/InternalResources/Icons/${icon_base_name}.icns")
		if(EXISTS ${icon_file})
			set(plist_file "${CMAKE_CURRENT_BINARY_DIR}/${target}_Info.plist")
			configure_file(
				"${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos_Info.plist.in"
				${plist_file}
				@ONLY
			)
			set_target_properties(${target} PROPERTIES
				MACOSX_BUNDLE_INFO_PLIST "${plist_file}"
				MACOSX_BUNDLE_ICON_FILE "${icon_base_name}.icns"
				MACOSX_BUNDLE_BUNDLE_NAME "${target}"
				MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
				MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
				)
			add_custom_command(TARGET ${target} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E make_directory
					"$<TARGET_BUNDLE_CONTENT_DIR:${target}>/InternalResources/${icon_base_name}.icns"
				COMMENT "Copying icon to macOS bundle"
			)
		else()
			message(WARNING "Icon file ${icon_file} not found!")
		endif()
	elseif(UNIX AND NOT APPLE)
		set(icon_file "${CMAKE_CURRENT_SOURCE_DIR}/InternalResources/Icons/${icon_base_name}.png")
        if(EXISTS "${icon_file}")
            set(desktop_file "${CMAKE_CURRENT_BINARY_DIR}/${target}.desktop")
            configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/cmake/linux_desktop.in"
                "${desktop_file}"
                @ONLY
            )
            
            install(FILES "${desktop_file}"
                DESTINATION "share/applications"
                RENAME "${target}.desktop"
            )
            
            install(FILES "${icon_file}"
                DESTINATION "share/pixmaps"
                RENAME "${target}.png"
            )
        else()
            message(WARNING "Icon file not found: ${icon_file}")
        endif()
    endif()
endfunction()

# Export the function to subdirectories
set(SET_TARGET_ICON_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetTargetIcon.cmake")

# Add subdirectories
add_subdirectory(OtterEngine)
add_subdirectory(OtterStudio)
add_subdirectory(OtterPlayground)