#pragma once

#include <cstddef>
#include <format>
#include <iostream>
#include <string>

namespace tradestack {
    namespace utils {
        template<typename T>
        void printField(const std::string& label, const T& value, size_t labelWidth = 15,
                        std::ostream& os = std::cout) {
            os << std::format("{:<{}}: {}\n", label, labelWidth, value);
        }
    }  // namespace utils
}  // namespace tradestack
