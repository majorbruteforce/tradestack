#pragma once
#include <functional>
#include <string>
#include <vector>
#include "price_level_node.hpp"

#include <avl_tree.hpp>

class SideTree : public AVLTree<PriceLevelNode>
{
   protected:
    PriceLevelNode*                     root;
    PriceLevelNode*                     best;
    size_t                              orderCount;
    std::function<bool(double, double)> priceComparator;

   public:
    SideTree(std::function<bool(double, double)> cmp)
        : root(nullptr), best(nullptr), orderCount(0), priceComparator(std::move(cmp))
    {
    }

    virtual ~SideTree() = default;

    virtual PriceLevelNode*     insert(const Order& order);
    virtual PriceLevelNode*     remove(const std::string& orderId);
    virtual PriceLevelNode*     find(const std::string& orderId);
    virtual std::vector<Order*> top(int length = 1) const;

    size_t size() const { return orderCount; }
    bool   empty() const { return orderCount == 0; }

    void inorder(std::function<void(PriceLevelNode*)> func);
};
