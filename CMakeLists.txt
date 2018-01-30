cmake_minimum_required(VERSION 3.5)
project(ipool)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(deps/gtest)

include_directories(SYSTEM deps/gtest/googletest/include)

add_executable(ipool src/main.cpp src/Pool.cpp src/Pool.h src/test-ipool.cpp )

target_link_libraries(ipool gtest)
