#pragma once
#include <deque>

#include "order.hpp"

struct PriceLevelNode {
  uint64_t price;
  std::deque<Order> level;

  PriceLevelNode *left = nullptr;
  PriceLevelNode *right = nullptr;

  explicit PriceLevelNode(const uint64_t p) : price(p) {}
};
