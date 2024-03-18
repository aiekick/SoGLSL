FetchContent_Declare(
	picojson
	GIT_REPOSITORY	https://github.com/kazuho/picojson.git
	GIT_TAG			v1.3.0
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/picojson
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(picojson)
if(NOT picojson_POPULATED)
	FetchContent_Populate(picojson)
	
	set(PICOJSON_INCLUDE_DIR ${picojson_SOURCE_DIR})
endif()