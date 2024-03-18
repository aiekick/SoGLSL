message("USE OF SDL2 as GUI")

set(LIBC ON CACHE BOOL "" FORCE) # solve a conflict issue with msvc and memcpy already defined error
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sdl)

set_target_properties(SDL2-static SDL2main PROPERTIES FOLDER 3rdparty)

set(SDL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sdl/include)
set(SDL_LIBRARIES SDL2-static SDL2main)
	
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(SDL2main PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
	target_compile_options(SDL2-static PRIVATE -Wno-everything) # disable all warnings, since im not maintaining this lib
endif()

add_definitions(-DUSE_SDL2)
