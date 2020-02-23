cmake_minimum_required(VERSION 3.16)
project(PRT)

set(CMAKE_CXX_STANDARD 20)

add_executable(PRT main.cpp icosahedron.cpp)
target_link_libraries(PRT OpenCL SDL2)

add_executable(CLTest CLTest.cpp)
target_link_libraries(CLTest OpenCL)