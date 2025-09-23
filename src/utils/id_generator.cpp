#include "utils/id_generator.hpp"

#include <cstdint>
#include <mutex>
#include <random>

namespace {

// single RNG / distribution / mutex for the program
std::mt19937_64                         rng{std::random_device{}()};
std::uniform_int_distribution<uint64_t> dist;
std::mutex                              rng_mutex;

}  // namespace

std::string utils::IdGenerator::next() {
    // obtain a 64-bit random value under the lock
    uint64_t v;
    {
        std::lock_guard<std::mutex> lock(rng_mutex);
        v = dist(rng);
    }

    // convert to fixed-width 16-char lowercase hex (zero-padded)
    char buf[17];
    for (int i = 0; i < 16; ++i) {
        // take nibble from most-significant to least
        unsigned nibble = static_cast<unsigned>((v >> ((15 - i) * 4)) & 0xF);
        buf[i]          = (nibble < 10) ? static_cast<char>('0' + nibble)
                                        : static_cast<char>('a' + (nibble - 10));
    }
    buf[16] = '\0';
    return std::string(buf, 16);
}
