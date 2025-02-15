cmake_minimum_required(VERSION 3.10)
project(EX2)

add_library(ex2 STATIC ex2.cpp)
add_dependencies(ex2 block_group)
target_include_directories(ex2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})