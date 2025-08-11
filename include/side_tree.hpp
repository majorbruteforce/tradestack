#pragma once
#include "price_level_node.hpp"
#include "price_level_tree.hpp"

class SideTree : public PriceLevelTree
{
   private:
    PriceLevelNode *root       = nullptr;
    PriceLevelNode *best       = nullptr;
    size_t          orderCount = 0;
};