#pragma once

#include <cstddef>
#include <format>
#include <iostream>
#include <string>

/**
 * @file print_utils.hpp
 * @brief Utility functions for formatted printing of labeled fields.
 *
 * Provides a helper for consistent output formatting of fields (label + value pairs)
 * to an output stream. Uses `std::format` internally for alignment and styling.
 */

namespace tradestack {
    namespace utils {
        /**
         * @brief Print a labeled field to an output stream.
         *
         * This function prints a field in the format:
         * ```
         * Label          : Value
         * ```
         * where the label is left-aligned and padded to a specified width.
         *
         * Example:
         * ```cpp
         * tradestack::utils::printField("Price", 105.25);
         * // Output: "Price          : 105.25"
         * ```
         *
         * @tparam T Type of the value to print. Must be formattable with `std::format`.
         * @param label       The text label describing the field.
         * @param value       The value to print alongside the label.
         * @param labelWidth  Minimum width for the label column (default = 15).
         * @param os          Output stream (defaults to `std::cout`).
         */
        template<typename T>
        void printField(const std::string& label, const T& value, size_t labelWidth = 15,
                        std::ostream& os = std::cout) {
            os << std::format("{:<{}}: {}\n", label, labelWidth, value);
        }

    }  // namespace utils
}  // namespace tradestack
