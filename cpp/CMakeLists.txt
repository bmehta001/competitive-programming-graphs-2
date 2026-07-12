cmake_minimum_required(VERSION 3.16)

project(Graphs2Cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(q1_bellman_ford question1_bellman_ford.cpp)
target_compile_features(q1_bellman_ford PRIVATE cxx_std_17)
