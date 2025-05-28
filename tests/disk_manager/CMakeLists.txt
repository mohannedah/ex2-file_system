add_executable(disk_manager_test disk_manager_test.cpp)

target_link_libraries(disk_manager_test
    PRIVATE disk_manager gtest gtest_main
)

include(GoogleTest)

gtest_discover_tests(disk_manager_test)
