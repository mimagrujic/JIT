cmake_minimum_required(VERSION 3.30)
project(JIT)

set(CMAKE_CXX_STANDARD 20)

add_executable(JIT main.cpp
        include/jit-tools/jit-tools.h
        src/jit-tools/jit-tools.cpp)