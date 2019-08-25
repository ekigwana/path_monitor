# CMake.
cmake_minimum_required(VERSION 3.9.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)

# Project configuration.
option(CMAKE_PREFIX_PATHS "Additional CMake prefix paths")
option(WITH_EXAMPLE "Enable Example." ON)
option(WITH_TESTS "Enable Tests." ON)

set(PROJECT_NAME path_monitor)
project(${PROJECT_NAME} C CXX)
if(NOT CMAKE_PREFIX_PATHS)
	set(CMAKE_PREFIX_PATHS "")
else()
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_PREFIX_PATHS})
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/image" CACHE PATH "default installation path" FORCE)
endif()

# CMake helpers.
include(GNUInstallDirs)

add_subdirectory(path_monitor)

if(WITH_EXAMPLE)
	add_subdirectory(example)
endif()

if(WITH_TESTS)
	add_subdirectory(test)
endif()
