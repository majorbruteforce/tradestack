#!/usr/bin/env bash
set -e

BUILD_DIR="build"
TARGET="tradestack"

echo "Building project..."
cmake -S . -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"

echo "Running $TARGET..."
./"$BUILD_DIR"/"$TARGET"
