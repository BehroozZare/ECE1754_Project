cmake_minimum_required(VERSION 3.17)
project(test_functions)

set(CMAKE_CXX_STANDARD 11)

add_executable(rose_original_code rose_original_code.cpp)

add_executable(hand_made_code hand_made_code.cpp)

add_executable(original_code original_code.cpp)