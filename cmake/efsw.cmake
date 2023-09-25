set(VERBOSE OFF CACHE BOOL "" FORCE)
set(BUILD_TEST_APP OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

##EXCLUDE_FROM_ALL reject install for this target
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/efsw EXCLUDE_FROM_ALL)

set_target_properties(efsw PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(efsw PROPERTIES FOLDER 3rdparty)

set(EFSW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/efsw/include)
set(EFSW_LIBRARIES efsw)
