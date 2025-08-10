#pragma once
#include "price_level_node.hpp"
#include "price_level_tree.hpp"

class AskTree : public PriceLevelTree
{
   private:
    PriceLevelNode *root = nullptr;
    PriceLevelNode *best = nullptr;

   public:
    void insertOrder(const Order &order) override
    {
        if ( !root )
        {
            root = new PriceLevelNode(order.price);
        }
    }

    bool isEmpty() const override
    {
        return root == nullptr;
    }
};