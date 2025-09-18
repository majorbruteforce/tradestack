#pragma once
#include <sstream>
#include <string>
#include <vector>

static inline std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \r\t\n");
    if (a == std::string::npos)
        return "";
    size_t b = s.find_last_not_of(" \r\t\n");
    return s.substr(a, b - a + 1);
}

static inline std::vector<std::string> split_ws(const std::string &s) {
    std::istringstream       iss(s);
    std::vector<std::string> out;
    std::string              tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

static void to_upper(std::string &s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}

static std::string timepoint_to_string(std::chrono::system_clock::time_point tp) {
    if (tp == std::chrono::system_clock::time_point{})
        return "none";
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm     tm;
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    tm = *std::localtime(&t);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%F %T", &tm);
    return std::string(buf);
}