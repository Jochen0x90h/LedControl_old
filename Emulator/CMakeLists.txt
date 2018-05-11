cmake_minimum_required(VERSION 3.1)

# set project
project(Emulator)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)

find_package(OpenGL REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
#message("*** ${GLFW_LIBRARY_DIRS} ${GLFW_LIBRARIES}")

if(APPLE)
	# OSX specific GLFW dependencies
	find_library(COCOA Cocoa)
	find_library(IOKIT IOKit)
	find_library(COREVIDEO CoreVideo)
	set(GLFW_LIBRARIES
		${COCOA}
		${IOKIT}
		${COREVIDEO}
		${GLFW_LIBRARIES})
endif()

include_directories(
	${GLFW_INCLUDE_DIRS}
	.
	../Effects
	../Fonts)
link_directories(${GLFW_LIBRARY_DIRS})

set(EFFECTS
	../Effects/hsv2rgb.h
	../Effects/noise.h
	../Effects/Effect.h
	../Effects/Rainbow.h
	../Effects/Color.h
	../Effects/MovingLight.h
	../Effects/Autumn.h)
source_group(Effects FILES ${EFFECTS})

set(FONTS
	../Fonts/Font.h
	../Fonts/Font.cpp
	../Fonts/tahoma_8pt.h
	../Fonts/tahoma_8pt.cpp)
source_group(Fonts FILES ${FONTS})

add_executable(emulator
	main.cpp
	glad.c
	Bitmap.h
	Bitmap.cpp
	Display.h
	Display.cpp
	Widget.cpp
	Widget.h
	LedStrip.cpp
	LedStrip.h
	Slider.cpp
	Slider.h
	Types.h
	Util.h
	${EFFECTS}
	${FONTS})
target_link_libraries(emulator
	${OPENGL_gl_LIBRARY}
	${GLFW_LIBRARIES})


# execute generator before build
find_program(GENERATOR ledgenerator)

add_custom_target(generate
	COMMAND ${GENERATOR}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../Effects)
add_dependencies(emulator generate)
