cmake_minimum_required(VERSION 3.10)
project(shell_manager)

add_library(shell_manager STATIC shell_manager.cpp)
add_dependencies(shell_manager ex2)
target_include_directories(shell_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
