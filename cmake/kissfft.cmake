set(KISSFFT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/kissfft)

set(KISSFFT_DATATYPE "float" CACHE STRING "")
set(KISSFFT_PKGCONFIG OFF CACHE BOOL "")
set(KISSFFT_STATIC ON CACHE BOOL "")
set(KISSFFT_TEST OFF CACHE BOOL "")
set(KISSFFT_TOOLS OFF CACHE BOOL "")
set(KISSFFT_USE_ALLOCA OFF CACHE BOOL "")

find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    set(KISSFFT_OPENMP ON CACHE BOOL "")
else()
    set(KISSFFT_OPENMP OFF CACHE BOOL "")
    message("WARNING : OpenMp not found, kissfft will be slower")
endif()

add_subdirectory(${KISSFFT_INCLUDE_DIR} EXCLUDE_FROM_ALL)
include_directories(${KISSFFT_INCLUDE_DIR})    

set_target_properties(kissfft PROPERTIES LINKER_LANGUAGE C)
set_target_properties(kissfft PROPERTIES FOLDER 3rdparty/audio)

set(KISSFFT_LIBRARIES kissfft)
