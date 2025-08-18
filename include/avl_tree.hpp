#pragma once
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <queue>
template <typename NodeType>
class AVLTree {
   public:
    void      inorder(NodeType* root, std::function<void(NodeType*)> func, size_t limit);
    NodeType* insert(NodeType* root, uint64_t price, NodeType*& out);
    NodeType* remove(NodeType* root, uint64_t price);
    void      freeTree(NodeType* root);

    void printTree(NodeType* root);

    NodeType* findMin(NodeType* node);
    NodeType* findMax(NodeType* node);

    //    private:
    int height(NodeType* node);
    int balanceFactor(NodeType* node);

    NodeType* rotateLeft(NodeType* node);
    NodeType* rotateRight(NodeType* node);
    NodeType* balance(NodeType* node);
    void      updateHeight(NodeType* node);
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
void AVLTree<NodeType>::inorder(NodeType* root, std::function<void(NodeType*)> func, size_t limit) {
    size_t count = 0;

    std::function<void(NodeType*)> recurse = [&](NodeType* node) {
        if (!node || count >= limit)
            return;
        recurse(node->left);
        if (count < limit) {
            func(node);
            ++count;
        }
        recurse(node->right);
    };

    recurse(root);
}

template <typename NodeType>
NodeType* AVLTree<NodeType>::insert(NodeType* root, uint64_t price, NodeType*& out) {
    if (!root) {
        out = new NodeType(price);
        return out;
    }

    if (price < root->price) {
        root->left = insert(root->left, price, out);
    } else if (price > root->price) {
        root->right = insert(root->right, price, out);
    } else {
        out = root;
        return root;
    }

    return balance(root);
}

template <typename NodeType>
NodeType* AVLTree<NodeType>::remove(NodeType* root, uint64_t price) {
    if (!root)
        return nullptr;

    if (price < root->price) {
        root->left = remove(root->left, price);
    } else if (price > root->price) {
        root->right = remove(root->right, price);
    } else {
        if (!root->left || !root->right) {
            NodeType* temp = root->left ? root->left : root->right;

            if (!temp) {
                delete root;
                return nullptr;
            } else {
                NodeType* old = root;
                root          = temp;
                delete old;
            }
        } else {
            NodeType* successor = findMin(root->right);

            root->leanCopy(successor);
            root->right = remove(root->right, successor->price);
        }
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

template <typename NodeType>
void AVLTree<NodeType>::printTree(NodeType* root) {
    if (!root)
        return;

    auto findHeight = [&](NodeType* node) {
        if (!node)
            return -1;
        std::queue<NodeType*> q;
        int                   height = -1;
        q.push(node);
        while (!q.empty()) {
            height++;
            int size = q.size();
            while (size--) {
                NodeType* cur = q.front();
                q.pop();
                if (cur->left)
                    q.push(cur->left);
                if (cur->right)
                    q.push(cur->right);
            }
        }
        return height;
    };

    int height = findHeight(root);
    int rows   = height + 1;
    int cols   = std::pow(2, rows) - 1;

    std::vector<std::vector<std::string>> grid(rows, std::vector<std::string>(cols, " "));

    std::function<void(NodeType*, int, int)> fill = [&](NodeType* node, int row, int col) {
        if (!node)
            return;
        grid[row][col] = std::to_string(node->price);

        int offset = std::pow(2, height - row - 1);
        if (node->left)
            fill(node->left, row + 1, col - offset);
        if (node->right)
            fill(node->right, row + 1, col + offset);
    };

    fill(root, 0, (cols - 1) / 2);

    for (auto& row : grid) {
        for (auto& cell : row) std::cout << (cell.empty() ? " " : cell);
        std::cout << "\n";
    }
}

template <typename NodeType>
NodeType* AVLTree<NodeType>::findMin(NodeType* node) {
    if (!node)
        return nullptr;
    while (node->left) node = node->left;
    return node;
}

template <typename NodeType>
NodeType* AVLTree<NodeType>::findMax(NodeType* node) {
    if (!node)
        return nullptr;
    while (node->right) node = node->right;
    return node;
}
