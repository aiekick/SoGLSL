set(IAGP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/InAppGpuProfiler)
set(IAGP_LIBRARIES InAppGpuProfiler)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/InAppGpuProfiler)

if(USE_SHARED_LIBS)
	set_target_properties(InAppGpuProfiler PROPERTIES FOLDER Libs/Shared)
	set_target_properties(InAppGpuProfiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
	set_target_properties(InAppGpuProfiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
	set_target_properties(InAppGpuProfiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
else()
	set_target_properties(InAppGpuProfiler PROPERTIES FOLDER Libs/Static)
endif()

# possible with the help of cmake_policy(SET CMP0079 NEW)
target_include_directories(InAppGpuProfiler PRIVATE 
	${IMGUIPACK_INCLUDE_DIR}
	${IMGUIPACK_INCLUDE_DIR}/3rdparty/imgui
)
target_link_libraries(InAppGpuProfiler PRIVATE ${IMGUIPACK_LIBRARIES})
