cmake_minimum_required(VERSION 3.22)
project(tradestack)

set(CMAKE_CXX_STANDARD 20)

add_executable(tradestack src/main.cpp
        header/order.hpp
        header/price_level_node.hpp
        src/order.cpp
        header/utils/id_generator.hpp
        src/utils/id_generator.cpp
        header/utils/print_utils.hpp)
