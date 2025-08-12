set(TEST_SOURCES
    avl_tree.cpp
    side_tree.cpp
)

add_executable(unit_tests ${TEST_SOURCES})
target_include_directories(unit_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(unit_tests PRIVATE gtest gtest_main)

add_test(NAME unit_tests COMMAND unit_tests)
