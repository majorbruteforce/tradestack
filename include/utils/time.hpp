#pragma once
#include <chrono>
#include <string>

inline std::string now_str() {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return std::string(buf);
}
