FetchContent_Declare(
	stb
	GIT_REPOSITORY	https://github.com/nothings/stb.git
	GIT_COMMIT		f7f20f39fe4f206c6f19e26ebfef7b261ee59ee4 # last commit at 21/07/2024 since there is no tags
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/stb
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
	FetchContent_Populate(stb)
	
	set(STB_INCLUDE_DIR ${stb_SOURCE_DIR})
endif()