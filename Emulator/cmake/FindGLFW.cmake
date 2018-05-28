# Finds glfw (https://cmake.org/Wiki/CMake:How_To_Find_Libraries)
#
# This module defines:
# GLFW_INCLUDE_DIRS
# GLFW_LIBRARIES
#

find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h)
find_library(GLFW_LIBRARY NAMES glfw3)

# handle the QUIETLY and REQUIRED arguments and set GLFW_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW DEFAULT_MSG
	GLFW_INCLUDE_DIR
	GLFW_LIBRARY
)
mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)

set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
IF(APPLE)
	find_library(IOKIT NAMES IOKit)
	find_library(COREVIDEO NAMES CoreVideo)
	find_library(COCOA NAMES Cocoa)
	mark_as_advanced(IOKIT COREVIDEO COCOA)
	set(GLFW_LIBRARIES ${GLFW_LIBRARY} ${IOKIT} ${COREVIDEO} ${COCOA})
elseif(UNIX)
	find_library(x11 NAMES X11)
	find_library(xrandr NAMES Xrandr)
	find_library(xi NAMES Xi)
	find_library(xxf86vm NAMES Xxf86vm)
	find_library(xinerama NAMES Xinerama)
	find_library(xcursor NAMES Xcursor)
	find_library(gl NAMES GL)
	mark_as_advanced(x11 xrandr xi xxf86vm xinerama xcursor)
	set(GLFW_LIBRARIES ${GLFW_LIBRARY} ${x11} ${xrandr} ${xi} ${xxf86vm} ${xinerama} ${xcursor} ${gl})
endif()
