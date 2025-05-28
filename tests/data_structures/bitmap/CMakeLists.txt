add_executable(bitmap_test bitmap_test.cpp)

target_link_libraries(bitmap_test
    PRIVATE bitmap gtest gtest_main
)

include(GoogleTest)

gtest_discover_tests(bitmap_test)
