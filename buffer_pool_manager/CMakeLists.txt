cmake_minimum_required(VERSION 3.10)
project(BufferPool)

add_library(buffer_pool STATIC buffer_pool.cpp)

target_include_directories(buffer_pool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})