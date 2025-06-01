add_executable(linked_list_test linked_list_test.cpp)

target_link_libraries(linked_list_test
    PRIVATE linked_list gtest gtest_main
)

include(GoogleTest)

gtest_discover_tests(linked_list_test)
