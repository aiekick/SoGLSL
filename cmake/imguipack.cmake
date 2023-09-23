set(IMGUIPACK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ImGuiPack)
set(IMGUIPACK_LIBRARIES ImGuiPack)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ImGuiPack)

if(USE_SHARED_LIBS)
	set_target_properties(ImGuiPack PROPERTIES FOLDER Libs/Shared)
else()
	set_target_properties(ImGuiPack PROPERTIES FOLDER Libs/Static)
endif()

set_target_properties(ImGuiPack PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(ImGuiPack PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(ImGuiPack PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
