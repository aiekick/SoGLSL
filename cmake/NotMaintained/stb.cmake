FetchContent_Declare(
	stb
	GIT_REPOSITORY	https://github.com/nothings/stb.git
	GIT_TAG			013ac3beddff3dbffafd5177e7972067cd2b5083 # last commit at 31/05/2024 since there is no tags
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/stb
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
	FetchContent_Populate(stb)
	
	set(STB_INCLUDE_DIR ${stb_SOURCE_DIR})
endif()