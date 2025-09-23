#pragma once
#include <cstdint>
#include <string>

namespace utils {
/**
 * @brief Thread‑safe generator for pseudo‑random 64‑bit IDs.
 *        Collision probability is negligible for most back‑testing
 *        or intraday real‑time workloads.
 */
class IdGenerator {
   public:
    static std::string next();
};
}  // namespace utils
