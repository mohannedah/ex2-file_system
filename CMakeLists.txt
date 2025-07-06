cmake_minimum_required(VERSION 3.10)
project(MyProject)

enable_testing()

add_subdirectory(third_party/googletest)
add_subdirectory(disk_simulator_manager)
add_subdirectory(data_structures)
add_subdirectory(file_system_manager)
add_subdirectory(shell_manager)
add_subdirectory(tests)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE 
    block_group
    ex2
    disk_manager  
    bitmap        
    linked_list
    shell_manager    
    gtest
    gtest_main  
)

configure_file(${CMAKE_SOURCE_DIR}/file_image ${CMAKE_BINARY_DIR}/file_image COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/file_image ${CMAKE_BINARY_DIR}/tests/file_image COPYONLY)
target_include_directories(main PRIVATE ${CMAKE_BINARY_DIR}, gtest PRIVATE third_party/googltest/include)
