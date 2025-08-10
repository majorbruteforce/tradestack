cmake_minimum_required(VERSION 3.22) project(tradestack)

    set(CMAKE_CXX_STANDARD 20) set(CMAKE_CXX_STANDARD_REQUIRED ON)

        add_executable(tradestack src /
                       main.cpp

                           src /
                       utils / id_generator.cpp src / order.cpp)
