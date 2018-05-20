cmake_minimum_required(VERSION 3.1)

# set project
project(Generator)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS system filesystem)

include_directories(${Boost_INCLUDE_DIRS})

message("Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}")

add_executable(ledgenerator
	main.cpp)
target_link_libraries(ledgenerator
	${Boost_LIBRARIES})

install(TARGETS ledgenerator
	RUNTIME DESTINATION bin)
