set(PROJECT_NAME path_monitor_example)
project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0")
set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -O0")

find_package(Threads)

find_package(Boost 1.67 REQUIRED COMPONENTS system)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} Threads::Threads stdc++fs)
