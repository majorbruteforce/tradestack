#pragma once
#include <deque>

#include <order.hpp>

struct PriceLevelNode {
    uint64_t          price;
    std::deque<Order> level;

    uint64_t        height = 0;
    PriceLevelNode *left   = nullptr;
    PriceLevelNode *right  = nullptr;

    explicit PriceLevelNode(const uint64_t p) : price(p) {}
};