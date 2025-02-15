cmake_minimum_required(VERSION 3.10)
project(Bitmap)

add_library(bitmap STATIC bitmap.cpp)

target_include_directories(bitmap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})