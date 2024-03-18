FetchContent_Declare(
	miniaudio
	GIT_REPOSITORY	https://github.com/mackron/miniaudio.git
	GIT_TAG			0.11.21
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/miniaudio
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(miniaudio)
if(NOT miniaudio_POPULATED)
	FetchContent_Populate(miniaudio)
	
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

	set(MINIAUDIO_LIBRARIES miniaudio)
	set(MINIAUDIO_INCLUDE_DIR ${miniaudio_SOURCE_DIR}/extras/miniaudio_split)

	file(GLOB MINIAUDIO_SOURCES 
		${MINIAUDIO_INCLUDE_DIR}/*.c
		${MINIAUDIO_INCLUDE_DIR}/*.h
	)

	if(USE_SHARED_LIBS)
		add_library(miniaudio ${MINIAUDIO_SOURCES})
		set_target_properties(miniaudio PROPERTIES FOLDER 3rdparty/Shared/audio)
	else()
		add_library(miniaudio STATIC ${MINIAUDIO_SOURCES})
		set_target_properties(miniaudio PROPERTIES FOLDER 3rdparty/Static/audio)
	endif()
	
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(miniaudio PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
	endif()
    
	set_target_properties(miniaudio PROPERTIES LINKER_LANGUAGE CXX)
endif()
