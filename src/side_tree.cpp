#include <functional>

#include <side_tree.hpp>

PriceLevelNode* SideTree::insert(const Order& order)
{
}

PriceLevelNode* SideTree::remove(const std::string& orderId)
{
}

PriceLevelNode* SideTree::find(const std::string& orderId)
{
}

std::vector<Order*> SideTree::top(int length = 1) const
{
}

void SideTree::inorder(std::function<void(PriceLevelNode*)> func)
{
    std::function<void(PriceLevelNode*)> recurse = [&](PriceLevelNode* node) {
        if (!node) return;
        recurse(node->left);
        func(node);
        recurse(node->right);
    };
    recurse(root);
}