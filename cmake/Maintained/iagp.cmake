set(IN_APP_GPU_PROFILER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/InAppGpuProfiler)
set(IN_APP_GPU_PROFILER_LIBRARIES InAppGpuProfiler)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/InAppGpuProfiler)

if(USE_SHARED_LIBS)
	set_target_properties(InAppGpuProfiler PROPERTIES FOLDER 3rdparty/aiekick/Shared)
	target_compile_definitions(ImGuiPack INTERFACE BUILD_CTOOLS_SHARED_LIBS)	
	target_compile_definitions(InAppGpuProfiler PRIVATE BUILD_IMGUI_PACK_SHARED_LIBS)
else()
	set_target_properties(InAppGpuProfiler PROPERTIES FOLDER 3rdparty/aiekick/Static)
endif()

target_include_directories(InAppGpuProfiler PRIVATE 
	${IMGUIPACK_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIR}
	${CTOOLS_INCLUDE_DIR}
	${GLAD_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
)

target_link_libraries(InAppGpuProfiler
	${IMGUIPACK_LIBRARIES}
	${OPENGL_LIBRARIES}
	${CTOOLS_LIBRARIES}
	${GLAD_LIBRARIES}
	${GLFW_LIBRARIES}
)

set_target_properties(InAppGpuProfiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(InAppGpuProfiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(InAppGpuProfiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
set_target_properties(InAppGpuProfiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${FINAL_BIN_DIR}")
set_target_properties(InAppGpuProfiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${FINAL_BIN_DIR}")

set_target_properties(InAppGpuProfiler PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(InAppGpuProfiler PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(InAppGpuProfiler PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
set_target_properties(InAppGpuProfiler PROPERTIES LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${FINAL_BIN_DIR}")
set_target_properties(InAppGpuProfiler PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${FINAL_BIN_DIR}")

