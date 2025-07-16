#pragma once
#include <cstdint>

namespace utils
{
  /**
   * @brief Thread‑safe generator for pseudo‑random 64‑bit IDs.
   *        Collision probability is negligible for most back‑testing
   *        or intraday real‑time workloads.
   */
  class IdGenerator
  {
  public:
    static uint64_t next();
  };
} // namespace utils
