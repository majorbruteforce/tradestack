#pragma once
#include <list>
#include <order.hpp>

struct PriceLevelNode {
    double           price;
    std::list<Order *> level;

    uint64_t        height = 1;
    PriceLevelNode *left   = nullptr;
    PriceLevelNode *right  = nullptr;

    void leanCopy(const PriceLevelNode *other) {
        if (!other)
            return;
        price  = other->price;
        level  = other->level;
        height = other->height;
    }

    explicit PriceLevelNode(const uint64_t p) : price(p) {}
};