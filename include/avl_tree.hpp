#pragma once
#include <algorithm>
template <typename NodeType>
class AVLTree {
   public:
    int height(NodeType* node);
    int balanceFactor(NodeType* node);

    NodeType* rotateLeft(NodeType* node);
    NodeType* rotateRight(NodeType* node);
    NodeType* balance(NodeType* node);
    void      updateHeight(NodeType* node);

    void      inorder(NodeType* root, std::function<void(NodeType*)> func);
    NodeType* insert(NodeType* root, uint64_t price);
    NodeType* remove(NodeType* root, uint64_t price);
    void      freeTree(NodeType* root);
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

template <typename NodeType>
NodeType* AVLTree<NodeType>::balance(NodeType* node) {
    if (!node)
        return nullptr;

    updateHeight(node);
    int bf = balanceFactor(node);

    if (bf > 1) {
        if (balanceFactor(node->left) < 0)
            node->left = rotateLeft(node->left);
        return rotateRight(node);
    } else if (bf < -1) {
        if (balanceFactor(node->right) > 0)
            node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

template <typename NodeType>
void AVLTree<NodeType>::updateHeight(NodeType* node) {
    node->height = 1 + std::max(height(node->left), height(node->right));
}

template <typename NodeType>
void AVLTree<NodeType>::inorder(NodeType* root, std::function<void(NodeType*)> func) {
    std::function<void(NodeType*)> recurse = [&](NodeType* node) {
        if (!node)
            return;
        recurse(node->left);
        func(node);
        recurse(node->right);
    };
    recurse(root);
}

template <typename NodeType>
NodeType* AVLTree<NodeType>::insert(NodeType* root, uint64_t price) {
    if (!root)
        return new NodeType(price);

    if (price < root->price) {
        root->left = AVLTree<NodeType>::insert(root->left, price);
    } else if (price > root->price) {
        root->right = AVLTree<NodeType>::insert(root->right, price);
    } else {
        return root;
    }

    return balance(root);
}

template <typename NodeType>
void AVLTree<NodeType>::freeTree(NodeType* root) {
    if (!root)
        return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}