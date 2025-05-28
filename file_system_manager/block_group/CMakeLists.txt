cmake_minimum_required(VERSION 3.10)
project(blockgroup)

add_library(block_group STATIC block_group.cpp)

add_dependencies(block_group bitmap)

target_include_directories(block_group PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
