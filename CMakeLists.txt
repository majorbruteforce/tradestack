cmake_minimum_required(VERSION 3.22)
project(tradestack)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(cmake/UpateSubmodules.cmake)
include(CTest)

add_executable(tradestack
    src/main.cpp
    src/utils/id_generator.cpp
    src/order.cpp
)
include_directories(include)

enable_testing()
add_subdirectory(external/googletest)
add_subdirectory(tests)
