#pragma once

#include <avl_tree.hpp>
#include <functional>
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

    virtual NodeType*           insert(const Order& order);
    virtual NodeType*           remove(const std::string& orderId);
    virtual NodeType*           find(const std::string& orderId);
    virtual std::vector<Order*> top(int length = 1) const;

    size_t size() const { return orderCount; }
    bool   empty() const { return orderCount == 0; }

    void inorder(std::function<void(NodeType*)> func);
};

template <typename NodeType>
NodeType* SideTree<NodeType>::insert(const Order& order) {
    if (!root) {
        root = new NodeType(order.price);
        best = root;
        orderCount++;
        return root;
    }
    return nullptr;
}

template <typename NodeType>
NodeType* SideTree<NodeType>::remove(const std::string& orderId) {
    return nullptr;
}

template <typename NodeType>
NodeType* SideTree<NodeType>::find(const std::string& orderId) {
    return nullptr;
}

template <typename NodeType>
std::vector<Order*> SideTree<NodeType>::top(int length) const {
    return {};
}

template <typename NodeType>
void SideTree<NodeType>::inorder(std::function<void(NodeType*)> func) {
    std::function<void(NodeType*)> recurse = [&](NodeType* node) {
        if (!node) return;
        recurse(node->left);
        func(node);
        recurse(node->right);
    };
    recurse(root);
}
