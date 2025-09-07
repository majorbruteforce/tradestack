#include "utils/id_generator.hpp"

#include <mutex>
#include <random>

using namespace tradestack::utils;

namespace tradestack {
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist;
    std::mutex rng_mutex;

    uint64_t IdGenerator::next() {
        std::lock_guard lock(rng_mutex);
        return dist(rng);
    }
}  // namespace tradestack
