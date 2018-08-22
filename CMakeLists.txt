# CMake.
cmake_minimum_required(VERSION 3.9.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
if(WITH_EXAMPLE)
	add_subdirectory(example)
endif()

if(WITH_TESTS)
	add_subdirectory(test)
endif()
