#!/usr/bin/env bash

set -e

if [ "$1" == "all" ]; then
    echo "Formatting all C++ source/header files..."
    clang-format -i $(find src include -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \))
    echo "✅ All files formatted."
else
    if [ $# -eq 0 ]; then
        echo "Usage: $0 all | <file1> <file2> ..."
        exit 1
    fi
    echo "Formatting selected files..."
    clang-format -i "$@"
    echo "✅ Selected files formatted."
fi
