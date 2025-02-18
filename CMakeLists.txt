cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0091 NEW) ## for CMAKE_MSVC_RUNTIME_LIBRARY
cmake_policy(SET CMP0079 NEW) # for have linking between libs

set(PROJECT SoGLSL)

enable_language(C CXX)

project(${PROJECT} CXX)

option(USE_SDL "Enale SDL instead of GLFW" OFF)
option(USE_PROFILER_TRACY "Enale Tracy Profiler" OFF)
option(USE_NETWORK "Enable NETWORK (Shader Inport, Versions, Changelog)" ON)
option(USE_STD_FILESYSTEM "Enable std::fielsystem use for path and ImGuiFileDialog" ON)
option(USE_VR "Enable VR Backend via OpenXR" ON)

## for group smake targets in the dir cmakeTargets
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmakeTargets")

if (USE_STD_FILESYSTEM)
	add_definitions(-DUSE_STD_FILESYSTEM)
	set(CMAKE_CXX_STANDARD 17)
else()
	set(CMAKE_CXX_STANDARD 14)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## some defines for debug mode (before 3rdparty.cmake)
if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") ## MT in release or MTd in debug
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG /Zi")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -g")
endif()

if(USE_VR)
add_definitions(-DUSE_VR)
endif()

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

set(3RDPARTY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)

set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)

include(cmake/3rdparty.cmake)

## some defines for debug mode
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
add_definitions(-DPROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

if(UNIX)
    if(APPLE)
        add_definitions(-DAPPLE)
    else()
        add_definitions(-DLINUX)
    endif()
    add_definitions(-DUNIX)
elseif(WIN32)
		add_definitions(-DWIN32)
	if(MINGW)
		add_definitions(-DMINGW32)
	endif()
    set(DIRENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/dirent/include)
    include_directories(${DIRENT_INCLUDE_DIR})
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DMSVC)
	add_definitions(-DNOMINMAX) ## for avoid issue regarding std::max and std::min
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-parameter")
endif ()

#needed by the class ExportBuffer
if (BUILD_X32)
	set(ARCH x32)
	add_definitions(-DVERSION_X32)
	message("${PROJECT} Build X32")
else()
	set(ARCH x64)
	add_definitions(-DVERSION_X64)
	message("${PROJECT} Build X64")
endif()

file(GLOB PROJECT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(main FILES ${PROJECT_MAIN})

file(GLOB_RECURSE PROJECT_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${PROJECT_SRC_RECURSE})

file(GLOB IMGUI_MISC_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ImGuiPack/imgui_docking/misc/cpp/imgui_stdlib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ImGuiPack/imgui_docking/misc/cpp/imgui_stdlib.h)
source_group(src/Imgui_Misc FILES ${IMGUI_MISC_SRC})

file(GLOB_RECURSE CMAKE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/cmake PREFIX cmake FILES ${CMAKE_RECURSE})

add_library(${PROJECT} STATIC
	${CMAKE_RECURSE} 
	${IMGUI_MISC_SRC}
	${PROJECT_SRC_RECURSE}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(${PROJECT} PRIVATE -Wno-unused-function) # its a lib, so a unused fucntion for an app can be used for an other app
endif()

if (USE_IMPLOT)
	target_compile_definitions(${PROJECT} PUBLIC USE_IMPLOT)
endif()
if (USE_IM_COOL_BAR)
	target_compile_definitions(${PROJECT} PUBLIC USE_IM_COOL_BAR)
endif()
if (USE_IM_GRADIENT_HDR)
	target_compile_definitions(${PROJECT} PUBLIC USE_IM_GRADIENT_HDR)
endif()
if (USE_IMGUI_FILE_DIALOG)
	target_compile_definitions(${PROJECT} PUBLIC USE_IMGUI_FILE_DIALOG)
endif()
if (USE_IM_GUIZMO)
	target_compile_definitions(${PROJECT} PUBLIC USE_IM_GUIZMO)
endif()
if (USE_IM_TOOLS)
	target_compile_definitions(${PROJECT} PUBLIC USE_IM_TOOLS)
endif()
if (USE_IMGUI_NODE_EDITOR)
	target_compile_definitions(${PROJECT} PUBLIC USE_IMGUI_NODE_EDITOR)
endif()
if (USE_IMGUI_MARKDOW)
	target_compile_definitions(${PROJECT} PUBLIC USE_IMGUI_MARKDOW)
endif()
if (USE_IMGUI_COLOR_TEXT_EDIT)
	target_compile_definitions(${PROJECT} PUBLIC USE_IMGUI_COLOR_TEXT_EDIT)
endif()
 
set(SOGSL_INCLUDE_DIRS
	${STB_INCLUDE_DIR}
	${SDL_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${GLAD_INCLUDE_DIR}
	${CURL_INCLUDE_DIR}
	${EFSW_INCLUDE_DIR}
	${PLOG_INCLUDE_DIR}
	${TRACY_INCLUDE_DIR}
	${IAGP_INCLUDE_DIRS}
	${RTMIDI_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
	${GLSLANG_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIRS}
	${OPENXR_INCLUDE_DIRS}
	${KISSFFT_INCLUDE_DIR}
	${PICOJSON_INCLUDE_DIR}
	${ALPHANUM_INCLUDE_DIR}
	${TINYXML2_INCLUDE_DIR}
	${MINIAUDIO_INCLUDE_DIR}
	${IMGUIPACK_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${IN_APP_GPU_PROFILER_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/tracy/public
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ezlibs/include
)

set(SOGSL_INCLUDE_DIRS ${SOGSL_INCLUDE_DIRS} PARENT_SCOPE) ## set this var in the parent scope


set(SOGLSL_LIBRARIES SoGLSL PARENT_SCOPE) ## set this var in the parent scope

include_directories(${SOGSL_INCLUDE_DIRS})

target_link_libraries(${PROJECT} PRIVATE
	${SDL_LIBRARIES}
	${GLM_LIBRARIES}
	${GLAD_LIBRARIES}
	${GLFW_LIBRARIES}
	${EFSW_LIBRARIES}
	${CURL_LIBRARIES}
	${IAGP_LIBRARIES}
	${TRACY_LIBRARIES}
	${OPENGL_LIBRARIES}
	${CTOOLS_LIBRARIES}
	${RTMIDI_LIBRARIES}
	${OPENXR_LIBRARIES}
	${ASSIMP_LIBRARIES}
	${KISSFFT_LIBRARIES}
	${GLSLANG_LIBRARIES}
	${TINYXML2_LIBRARIES}
	${MINIAUDIO_LIBRARIES}
	${IMGUIPACK_LIBRARIES}
	${IN_APP_GPU_PROFILER_LIBRARIES}
)

set(CTOOLS_LIBRARIES ${CTOOLS_LIBRARIES} PARENT_SCOPE)
