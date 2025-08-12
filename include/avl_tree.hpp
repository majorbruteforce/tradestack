#pragma once
#include <algorithm>
template <typename NodeType>
class AVLTree {
   public:
    NodeType* rotateLeft(NodeType* node);
    NodeType* rotateRight(NodeType* node);
    int       height(NodeType* node);
    int       balanceFactor(NodeType* node);
    void      updateHeight(NodeType* node);

    AVLTree() {}
    virtual ~AVLTree() = default;
};

template <typename NodeType>
int AVLTree<NodeType>::height(NodeType* node) {
    return node ? node->height : 0;
}

template <typename NodeType>
int AVLTree<NodeType>::balanceFactor(NodeType* node) {
    return height(node->left) - height(node->right);
}

template <typename NodeType>
void AVLTree<NodeType>::updateHeight(NodeType* node) {
    node->height = 1 + std::max(height(node->left), height(node->right));
}

template <typename NodeType>
NodeType* AVLTree<NodeType>::rotateLeft(NodeType* node) {
    NodeType* newRoot      = node->right;
    NodeType* movedSubtree = newRoot->left;

    newRoot->left = node;
    node->right   = movedSubtree;

    updateHeight(node);
    updateHeight(newRoot);

    return newRoot;
}

template <typename NodeType>
NodeType* AVLTree<NodeType>::rotateRight(NodeType* node) {
    NodeType* newRoot      = node->left;
    NodeType* movedSubtree = newRoot->right;

    newRoot->right = node;
    node->left     = movedSubtree;

    updateHeight(node);
    updateHeight(newRoot);

    return newRoot;
}