FetchContent_Declare(
	rtmidi
	GIT_REPOSITORY	https://github.com/thestk/rtmidi.git
	GIT_TAG			6.0.0
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/rtmidi
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(rtmidi)
if(NOT rtmidi_POPULATED)
	FetchContent_Populate(rtmidi)
	
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

	set(RTMIDI_LIBRARIES rtmidi)
	set(RTMIDI_INCLUDE_DIR ${rtmidi_SOURCE_DIR})

	set(BUILD_TESTING OFF CACHE BOOL "")

	##EXCLUDE_FROM_ALL reject install for this target
	add_subdirectory(${RTMIDI_INCLUDE_DIR} EXCLUDE_FROM_ALL)

	if(USE_SHARED_LIBS)
		set_target_properties(rtmidi PROPERTIES FOLDER 3rdparty/Shared/audio)
	else()
		set_target_properties(rtmidi PROPERTIES FOLDER 3rdparty/Static/audio)
	endif()
	
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(rtmidi PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
	endif()
endif()
