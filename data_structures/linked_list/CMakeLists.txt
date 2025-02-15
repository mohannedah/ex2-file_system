cmake_minimum_required(VERSION 3.10)
project(LinkedList)

add_library(linked_list INTERFACE)

target_include_directories(linked_list INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})