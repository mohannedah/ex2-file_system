cmake_minimum_required(VERSION 3.10)
project(MyProject)

add_subdirectory(disk_simulator_manager)
add_subdirectory(data_structures)
add_subdirectory(buffer_pool_manager)
add_subdirectory(file_system_manager)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE 
    block_group
    ex2
    disk_manager  
    bitmap        
    linked_list   
    buffer_pool   
)

target_include_directories(main PRIVATE ${CMAKE_BINARY_DIR})
