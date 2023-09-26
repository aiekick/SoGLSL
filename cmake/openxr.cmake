set(BUILD_FORCE_GENERATION ON CACHE BOOL "" FORCE)
set(BUILD_LOADER ON CACHE BOOL "" FORCE)
set(DYNAMIC_LOADER OFF CACHE BOOL "" FORCE)

set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)

set(OPENXR_STATIC ON CACHE BOOL "")
set(_oxr_build_type static CACHE STRING "static")
set(DYNAMIC_LOADER OFF CACHE BOOL "")

## seul moyen trouver pour desactiver vulkan dans openxr
## sinon il veut compiler avec vulkan et on a aucun moyen de lui dire non
set(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE "" CACHE STRING "" FORCE)
set(Vulkan_GLSLC_EXECUTABLE "" CACHE STRING "" FORCE)
set(Vulkan_INCLUDE_DIR "" CACHE STRING "" FORCE)
set(Vulkan_LIBRARY "" CACHE STRING "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/OpenXR_SDK EXCLUDE_FROM_ALL)
set(OPENXR_LIBRARIES openxr_loader)
set(OPENXR_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/OpenXR_SDK/include)
set_target_properties(openxr_loader PROPERTIES FOLDER 3rdparty/OpenXR)
set_target_properties(generate_openxr_header PROPERTIES FOLDER 3rdparty/OpenXR)
set_target_properties(xr_global_generated_files PROPERTIES FOLDER 3rdparty/OpenXR)

add_definitions(-DUSE_VR)
add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)

if(WIN32)
	add_definitions(-DXR_USE_PLATFORM_WIN32)
else(UNIX AND NOT APPLE)
	add_definitions(-DXR_USE_PLATFORM_LINUX)
endif()
