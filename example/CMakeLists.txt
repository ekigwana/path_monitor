
set(PROJECT path_monitor)
project(${PROJECT})

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0")
set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -O0")

find_package(Threads)

find_package(Boost 1.67 REQUIRED COMPONENTS system)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT} main.cpp)

target_link_libraries(${PROJECT} Threads::Threads stdc++fs)
