#!/usr/bin/env bash

set -euo pipefail

BUILD_DIR="build"
TEST_COLOR="${TEST_COLOR:-1}"

usage() {
    echo "Usage:"
    echo "  $0              # build and run all tests"
    echo "  $0 <test_name>  # build and run a specific test"
    echo "  $0 clean        # remove build directory"
    echo "  TEST_COLOR=0 $0 # disable gtest colors"
    exit 1
}

main() {
    if [[ $# -gt 0 && "$1" == "clean" ]]; then
        echo "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
        exit 0
    fi

    if [[ ! -d "$BUILD_DIR" ]]; then
        echo "Creating build directory..."
        mkdir -p "$BUILD_DIR"
    fi

    echo "Configuring project with CMake..."
    cmake -S . -B "$BUILD_DIR"

    echo "Building project..."
    cmake --build "$BUILD_DIR" --parallel

    echo "Running tests..."
    if [[ $# -eq 0 ]]; then
        echo "===== Running all tests ====="
        GTEST_COLOR="$TEST_COLOR" ctest --test-dir "$BUILD_DIR" --output-on-failure
    else
        for test_name in "$@"; do
            echo -e "\n===== Running only: $test_name ====="
            GTEST_COLOR="$TEST_COLOR" ctest --test-dir "$BUILD_DIR" -R "^${test_name}$" --output-on-failure
        done
    fi
}

main "$@"
