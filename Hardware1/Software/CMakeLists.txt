cmake_minimum_required(VERSION 3.1)

# find avr tools (inspired by https://github.com/mkleemann/cmake-avr)
find_program(AVR_CC avr-gcc)
find_program(AVR_CXX avr-g++)
find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_SIZE avr-size)
find_program(AVR_OBJDUMP avr-objdump)
find_program(AVRDUDE avrdude)

# enable cross compiling
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1.0.0)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})


# project
project(LedControl)

# config
set(PROGRAMMER avrisp2)
set(PROGRAMMER_PORT /dev/tty.usbmodem1421)
set(MCU atmega328p)

# fuses CKDIV8: off, EESAVE: on, BODLEVEL: 2.7V (http://www.engbedded.com/fusecalc)
set(LFUSE 0xe2)
set(HFUSE 0xd1)
set(EFUSE 0xfd)


# default build type
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# compiler options for all build types
add_definitions(
	-Wall
	-DNO_NAMES)

include_directories(
	.
	../../Effects)

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
	../../Effects/Plasma.h)

# executable
add_executable(ledcontrol.elf
	main.c
	util.h
	${EFFECTS})
set_target_properties(ledcontrol.elf PROPERTIES
	COMPILE_FLAGS "-mmcu=${MCU} -ffunction-sections -fdata-sections"
	LINK_FLAGS "-mmcu=${MCU} -Wl,--gc-sections"
)

# upload
add_custom_target(
	upload
	${AVRDUDE} -c ${PROGRAMMER} -p ${MCU} -P ${PROGRAMMER_PORT}
		-U flash:w:ledcontrol.elf:e
	DEPENDS ${hex_file}
	COMMENT "Upload to device"
)

# fuses
add_custom_target(
	fuses
	${AVRDUDE} -c ${PROGRAMMER} -p ${MCU} -P ${PROGRAMMER_PORT}
		-U lfuse:w:${LFUSE}:m
		-U hfuse:w:${HFUSE}:m
		-U efuse:w:${EFUSE}:m
	COMMENT "Set fuses of device"
)

# size
add_custom_target(
	size
	${AVR_SIZE} --mcu=${MCU} -C ledcontrol.elf
	COMMENT "Print size of program and data"
)
