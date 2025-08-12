#pragma once

template <typename NodeType>
class AVLTree
{
   public:
    NodeType* rotateLeft(NodeType* node);
    NodeType* rotateRight(NodeType* node);
    int       height(NodeType* node);
    int       balanceFactor(NodeType* node);
    void      updateHeight(NodeType* node);

    AVLTree() {}
    virtual ~AVLTree() = default;
};