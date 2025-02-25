cmake_minimum_required(VERSION 3.10)
project(Blockgroup)

add_library(block_group STATIC block_group.cpp)

add_dependencies(block_group bitmap)

target_include_directories(block_group PUBLIC ${PROJECT_SOURCE_DIR})