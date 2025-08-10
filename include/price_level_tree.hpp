#pragma once
#include <vector>

#include "order.hpp"

class PriceLevelTree
{
   public:
    virtual void                      insertOrder(const Order& order)         = 0;
    virtual void                      removeOrder(const std::string& orderId) = 0;
    virtual const std::vector<Order*> getTop(const int& length = 1) const     = 0;
    virtual bool                      isEmpty() const                         = 0;
    virtual ~PriceLevelTree()                                                 = default;
};
