cmake_minimum_required(VERSION 3.8)

set(PROJECT path_monitor_tests)
project(${PROJECT})

enable_testing()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-ggdb3 -O0")
set(CMAKE_C_FLAGS "-ggdb3 -O0")

# find_package(Threads)
find_package(GTest REQUIRED)
find_package(Boost 1.67 REQUIRED COMPONENTS thread unit_test_framework)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(sync sync.cpp)
target_link_libraries(sync Boost::thread Boost::unit_test_framework GTest::Main stdc++fs)
add_test(TestSYNC sync)


add_executable(async async.cpp)
target_link_libraries(async Boost::thread Boost::unit_test_framework GTest::Main stdc++fs)
add_test(TestASYNC async)
