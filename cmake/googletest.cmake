if (USE_TEST)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/googletest)
set_target_properties(gtest PROPERTIES FOLDER 3rdparty/googletest)
set_target_properties(gtest_main PROPERTIES FOLDER 3rdparty/googletest)
endif()