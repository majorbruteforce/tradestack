# AVL Tree tests
add_executable(avl_tree_tests avl_tree.cpp)
target_include_directories(avl_tree_tests PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(avl_tree_tests PRIVATE gtest gtest_main)
add_test(NAME avl_tree_tests COMMAND avl_tree_tests)

# Side Tree tests
add_executable(side_tree_tests side_tree.cpp)
target_include_directories(side_tree_tests PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(side_tree_tests PRIVATE gtest gtest_main)
add_test(NAME side_tree_tests COMMAND side_tree_tests)
