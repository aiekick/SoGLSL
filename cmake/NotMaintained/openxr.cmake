FetchContent_Declare(
	openxr
	GIT_REPOSITORY	https://github.com/KhronosGroup/OpenXR-SDK-Source.git
	GIT_TAG			release-1.0.34
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/openxr
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(openxr)
if(NOT openxr_POPULATED)
	FetchContent_Populate(openxr)
	
	if(USE_SHARED_LIBS)
		set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
		set(LLVM_USE_CRT_DEBUG MDd CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_MINSIZEREL MD CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELEASE MD CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELWITHDEBINFO MD CACHE STRING "" FORCE)
		set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "")
		set(USE_STATIC_CRT OFF CACHE BOOL "" FORCE)
	else()
		set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
		set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
		set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
		set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "")
		set(USE_STATIC_CRT ON CACHE BOOL "" FORCE)
	endif()
	
	if(NOT CMAKE_DEBUG_POSTFIX)
	  set(CMAKE_DEBUG_POSTFIX _debug)
	endif()
	if(NOT CMAKE_RELEASE_POSTFIX)
	  set(CMAKE_RELEASE_POSTFIX)
	endif()
	if(NOT CMAKE_MINSIZEREL_POSTFIX)
	  set(CMAKE_MINSIZEREL_POSTFIX _minsizerel)
	endif()
	if(NOT CMAKE_RELWITHDEBINFO_POSTFIX)
	  set(CMAKE_RELWITHDEBINFO_POSTFIX _reldeb)
	endif()

	set(OPENXR_LIBRARIES openxr_loader)
	set(OPENXR_INCLUDE_DIR ${OPENXR_SOURCE_DIR}/include)

	set(BUILD_LOADER ON CACHE BOOL "" FORCE)
	set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(BUILD_API_LAYERS OFF CACHE BOOL "" FORCE)
	set(BUILD_FORCE_GENERATION ON CACHE BOOL "" FORCE)
	set(BUILD_CONFORMANCE_TESTS OFF CACHE BOOL "" FORCE)

	set(DYNAMIC_LOADER OFF CACHE BOOL "" FORCE)
	set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
	set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
	set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
	set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
	set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "")
	
	if(USE_SHARED_LIBS)
		set(DYNAMIC_LOADER ON CACHE BOOL "")
		set(OPENXR_STATIC OFF CACHE BOOL "")
		set(_oxr_build_type shared CACHE STRING "shared")
	else()
		set(DYNAMIC_LOADER OFF CACHE BOOL "")
		set(OPENXR_STATIC ON CACHE BOOL "")
		set(_oxr_build_type static CACHE STRING "static")
	endif()

	if (USE_OPENGL)
		## only way found for disable vulkan when we target opengl
		set(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE "" CACHE STRING "" FORCE)
		set(Vulkan_GLSLC_EXECUTABLE "" CACHE STRING "" FORCE)
		set(Vulkan_INCLUDE_DIR "" CACHE STRING "" FORCE)
		set(Vulkan_LIBRARY "" CACHE STRING "" FORCE)
	endif()

	##EXCLUDE_FROM_ALL reject install for this target
	add_subdirectory(${openxr_SOURCE_DIR} EXCLUDE_FROM_ALL)

	if(USE_SHARED_LIBS)
		set_target_properties(openxr_loader PROPERTIES FOLDER 3rdparty/Shared/OpenXR)
		set_target_properties(generate_openxr_header PROPERTIES FOLDER 3rdparty/Shared/OpenXR)
		set_target_properties(xr_global_generated_files PROPERTIES FOLDER 3rdparty/Shared/OpenXR)
	else()
		set_target_properties(openxr_loader PROPERTIES FOLDER 3rdparty/Static/OpenXR)
		set_target_properties(generate_openxr_header PROPERTIES FOLDER 3rdparty/Static/OpenXR)
		set_target_properties(xr_global_generated_files PROPERTIES FOLDER 3rdparty/Static/OpenXR)
	endif()

	add_definitions(-DUSE_VR)

	if (USE_OPENGL)
		add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)
	endif()

	if (USE_VULKAN)
		add_definitions(-DXR_USE_GRAPHICS_API_VULKAN)
	endif()
	
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(openxr_loader PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
		target_compile_options(generate_openxr_header PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
		target_compile_options(xr_global_generated_files PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
	endif()

	if(WIN32)
		add_definitions(-DXR_USE_PLATFORM_WIN32)
	endif()
endif()
