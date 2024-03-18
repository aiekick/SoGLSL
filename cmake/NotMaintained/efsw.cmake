FetchContent_Declare(
	efsw
	GIT_REPOSITORY	https://github.com/SpartanJ/efsw.git
	GIT_TAG			1.3.1
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/efsw
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(efsw)
if(NOT efsw_POPULATED)
	FetchContent_Populate(efsw)
	
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
	
	set(VERBOSE OFF CACHE BOOL "" FORCE)

	set(BUILD_TEST_APP OFF CACHE BOOL "" FORCE)
	
	##EXCLUDE_FROM_ALL reject install for this target
	add_subdirectory(${efsw_SOURCE_DIR} EXCLUDE_FROM_ALL)
	
	if(USE_SHARED_LIBS)
		target_compile_definitions(efsw INTERFACE BUILD_SHARED_LIBS)
		set_target_properties(efsw PROPERTIES POSITION_INDEPENDENT_CODE ON)
		set_target_properties(efsw PROPERTIES DEFINE_SYMBOL "EFSW_EXPORT")
		set_target_properties(efsw PROPERTIES FOLDER 3rdparty/Shared)
		set_target_properties(efsw PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
		set_target_properties(efsw PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
		set_target_properties(efsw PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
	else()
		set_target_properties(efsw PROPERTIES FOLDER 3rdparty/Static)
	endif()	
	
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(efsw PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
	endif()

	set(EFSW_INCLUDE_DIR ${efsw_SOURCE_DIR}/include)
	set(EFSW_LIBRARIES efsw)
endif()
