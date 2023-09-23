message("USE OF SDL2 as GUI")

set(LIBC ON CACHE BOOL "" FORCE) # solve a conflict issue with msvc and memcpy already defined error
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sdl)

set_target_properties(SDL2-static SDL2main PROPERTIES FOLDER 3rdparty)

set(SDL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sdl/include)
set(SDL_LIBRARIES SDL2-static SDL2main)

add_definitions(-DUSE_SDL2)
