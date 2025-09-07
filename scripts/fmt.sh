#!/usr/bin/env bash
set -euo pipefail

SRC_DIRS=("src" "include")

format_all() {
    echo "🔧 Formatting all C++ source/header files..."
    mapfile -d '' files < <(find "${SRC_DIRS[@]}" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -print0)
    if [ ${#files[@]} -eq 0 ]; then
        echo "⚠️  No files found."
        return
    fi
    clang-format -i "${files[@]}"
    echo "✅ Formatted ${#files[@]} files."
}

format_selected() {
    echo "🔧 Formatting selected files..."
    clang-format -i "$@"
    echo "✅ Formatted $# files."
}

if [ $# -eq 0 ] || [ "$1" = "all" ]; then
    format_all
else
    format_selected "$@"
fi
