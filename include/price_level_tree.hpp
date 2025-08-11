#pragma once
#include <vector>

#include "order.hpp"
#include "price_level_node.hpp"

class PriceLevelTree
{
   public:
    virtual PriceLevelNode*           insert(const Order& order)         = 0;
    virtual PriceLevelNode*           remove(const std::string& orderId) = 0;
    virtual PriceLevelNode*           find(const std::string& orderId)   = 0;
    virtual const std::vector<Order*> top(const int& length = 1) const   = 0;
    virtual size_t                    size()                             = 0;
    virtual bool                      empty() const                      = 0;
    virtual ~PriceLevelTree()                                            = default;
};
