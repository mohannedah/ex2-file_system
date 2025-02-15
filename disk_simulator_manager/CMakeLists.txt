cmake_minimum_required(VERSION 3.10)
project(DiskManager)

add_library(disk_manager STATIC disk.cpp)

target_include_directories(disk_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})