#pragma once

#include <avl_tree.hpp>
#include <functional>
#include <price_level_node.hpp>
#include <string>
#include <vector>

template <typename NodeType>
class SideTree {
   public:
    AVLTree<NodeType>                   avl;
    NodeType*                           root;
    NodeType*                           best;
    size_t                              orderCount;
    std::function<bool(double, double)> priceComparator;

    explicit SideTree(std::function<bool(double, double)> cmp)
        : root(nullptr), best(nullptr), orderCount(0), priceComparator(std::move(cmp)) {}

    virtual ~SideTree() = default;

    virtual NodeType*           insert(Order& order);
    virtual NodeType*           remove(Order& order);
    virtual NodeType*           find(const int& price);
    virtual std::vector<Order*> top(int length = 1) const;

    size_t size() const { return orderCount; }
    bool   empty() const { return orderCount == 0; }
};

template <typename NodeType>
NodeType* SideTree<NodeType>::insert(Order& order) {
    uint64_t price = order.price;

    if (!root) {
        root = new NodeType(price);
        best = root;
        orderCount++;
        order.level_posn = root->level.insert(root->level.end(), &order);

        return root;
    }

    NodeType* found = find(price);
    if (found) {
        order.level_posn = found->level.insert(found->level.end(), &order);
        return found;
    }

    NodeType* inserted = nullptr;
    NodeType* newRoot  = avl.insert(root, price, inserted);
    root               = newRoot;

    order.level_posn = inserted->level.insert(inserted->level.end(), &order);
    return inserted;
}

template <typename NodeType>
NodeType* SideTree<NodeType>::remove(Order& order) {
    int       price = order.price;
    NodeType* found = find(price);

    if (!found) {
        return nullptr;
    }

    found->level.erase(order.level_posn);
    orderCount--;
    return found;
}

template <typename NodeType>
NodeType* SideTree<NodeType>::find(const int& price) {
    if (!root)
        return nullptr;

    NodeType* node = root;

    while (node) {
        if (price > node->price) {
            node = node->right;
        } else if (price < node->price) {
            node = node->left;
        } else {
            return node;
        }
    }

    return nullptr;
}

template <typename NodeType>
std::vector<Order*> SideTree<NodeType>::top(int length) const {
    return {};
}
