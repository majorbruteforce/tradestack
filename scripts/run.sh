#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
TARGET="tradestack"

CORES=$(command -v nproc >/dev/null && nproc || sysctl -n hw.ncpu)

echo "Building project using $CORES cores..."
cmake -S . -B "$BUILD_DIR"

if [ -f "$BUILD_DIR/Makefile" ]; then
    cmake --build "$BUILD_DIR" -- -j"$CORES"
else
    cmake --build "$BUILD_DIR"
fi

echo "Running $TARGET..."
"./$BUILD_DIR/$TARGET"
