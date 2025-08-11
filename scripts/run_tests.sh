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
GTEST_COLOR="$TEST_CLR" ctest --output-on-failure
