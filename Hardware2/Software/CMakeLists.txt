cmake_minimum_required(VERSION 3.1)

# enable cross compiling
include(CMakeForceCompiler)
set(CMAKE_SYSTEM_NAME Generic)
CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)


# project
project(LedControl)

# config
set(FAMILY STM32L4)
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")


# default build type
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# place libopencm3 into LedControl root directory
# https://github.com/libopencm3
find_library(OPENCM3 opencm3_stm32l4 "${CMAKE_CURRENT_SOURCE_DIR}/../../libopencm3/lib")

# compiler options for all build types
add_definitions(-Wall)

include_directories(
	.
	../../Effects
	../../Display
	../../libopencm3/include
)

set(EFFECTS
	../../Effects/types.h
	../../Effects/hsv2rgb.h
	../../Effects/noise.h
	../../Effects/Effect.h
	../../Effects/Rainbow.h
	../../Effects/Color.h
	../../Effects/MovingLight.h
	../../Effects/Spring.h
	../../Effects/Summer.h
	../../Effects/Autumn.h
	../../Effects/Winter.h
	../../Effects/Plasma.h
)

set(DISPLAY
	../../Display/Bitmap.h
	../../Display/Bitmap.cpp
	../../Display/Font.h
	../../Display/Font.cpp
	../../Display/tahoma_8pt.h
	../../Display/tahoma_8pt.cpp
)

# executable
add_executable(ledcontrol
	main.cpp
	util.h
	Pin.h
	DmaChannel.h
	${EFFECTS}
	${DISPLAY}
)
set_target_properties(ledcontrol PROPERTIES
	COMPILE_FLAGS "-D${FAMILY} ${CPU_FLAGS} -ffunction-sections -fdata-sections"
	LINK_FLAGS "${CPU_FLAGS} --specs=nosys.specs -Wl,--gc-sections"
)
target_link_libraries(ledcontrol ${OPENCM3})

# upload
#add_custom_target(
#	upload
#	${AVRDUDE} -c ${PROGRAMMER} -p ${MCU} -P ${PROGRAMMER_PORT}
#		-U flash:w:ledcontrol.elf:e
#	DEPENDS ${hex_file}
#	#COMMENT "Uploading ${hex_file} to ${AVR_MCU} using ${AVR_PROGRAMMER}"
#	COMMENT "Uploading..."
#)
