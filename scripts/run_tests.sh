#!/usr/bin/env bash
set -e

BUILD_DIR="build"
TEST_CLR="1"

if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cmake -S . -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"

cd "$BUILD_DIR"

if [ $# -eq 0 ]; then
    echo "===== Running all tests ====="
    GTEST_COLOR="$TEST_CLR" ctest --output-on-failure
else
    for test_name in "$@"; do
        echo -e "\n===== Running only: $test_name ====="
        GTEST_COLOR="$TEST_CLR" ctest -R "^${test_name}$" --output-on-failure
    done
fi
