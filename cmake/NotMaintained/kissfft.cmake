FetchContent_Declare(
	kissfft
	GIT_REPOSITORY	https://github.com/mborgerding/kissfft.git
	GIT_TAG			131.1.0
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/kissfft
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(kissfft)
if(NOT kissfft_POPULATED)
	FetchContent_Populate(kissfft)
	
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
	
	set(KISSFFT_DATATYPE "float" CACHE STRING "")
	set(KISSFFT_PKGCONFIG OFF CACHE BOOL "")
	set(KISSFFT_STATIC ON CACHE BOOL "")
	set(KISSFFT_TEST OFF CACHE BOOL "")
	set(KISSFFT_TOOLS OFF CACHE BOOL "")
	set(KISSFFT_USE_ALLOCA OFF CACHE BOOL "")
	set(KISSFFT_OPENMP OFF CACHE BOOL "")

	set(KISSFFT_LIBRARIES kissfft)
	set(KISSFFT_INCLUDE_DIR ${kissfft_SOURCE_DIR})

	##EXCLUDE_FROM_ALL reject install for this target
	add_subdirectory(${kissfft_SOURCE_DIR} EXCLUDE_FROM_ALL)

	if(USE_SHARED_LIBS)
		set_target_properties(kissfft PROPERTIES FOLDER 3rdparty/Shared/audio)
	else()
		set_target_properties(kissfft PROPERTIES FOLDER 3rdparty/Static/audio)
	endif()
	
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(kissfft PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
	endif()
endif()
