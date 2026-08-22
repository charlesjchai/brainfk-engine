cmake_minimum_required(VERSION 3.15)

project(brainfk-engine LANGUAGES CXX)

# Need c++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(brainfkrun
    src/main.cpp
    src/utils.cpp
)

target_include_directories(brainfkrun PRIVATE include)
