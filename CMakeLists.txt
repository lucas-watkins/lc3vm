cmake_minimum_required(VERSION 4.0)
project(lc3vm)

set(CMAKE_CXX_STANDARD 20)

add_executable(lc3vm src/main.cpp src/Opcodes.cpp)