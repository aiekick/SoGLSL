FetchContent_Declare(
	stb
	GIT_REPOSITORY	https://github.com/nothings/stb.git
	GIT_TAG			ae721c50eaf761660b4f90cc590453cdb0c2acd0 # alst commit at 13/02/2024 since there is no tags
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/stb
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
	FetchContent_Populate(stb)
	
	set(STB_INCLUDE_DIR ${stb_SOURCE_DIR})
endif()