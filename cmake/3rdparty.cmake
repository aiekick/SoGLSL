if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(X11 REQUIRED)
  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR "X11 Xi library is required")
  endif ()
endif ()

if(USE_SDL)
	include(cmake/sdl.cmake)
else()
	include(cmake/glfw.cmake)
endif()

if(USE_VR)
	include(cmake/openxr.cmake)
endif()

# order is important
include(cmake/glad.cmake)
include(cmake/tinyxml2.cmake)
include(cmake/ctools.cmake)

# order is not important
include(cmake/glm.cmake)
include(cmake/stb.cmake)
include(cmake/curl.cmake)
include(cmake/efsw.cmake)
include(cmake/assimp.cmake)
include(cmake/rtmidi.cmake)
include(cmake/kissfft.cmake)
include(cmake/miniaudio.cmake)
include(cmake/imguipack.cmake)

