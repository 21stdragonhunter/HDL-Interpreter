cmake_minimum_required(VERSION 3.2)
project(HDL)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES HDL.cpp)
add_executable(HDL ${SOURCE_FILES})