#include <avl_tree.hpp>
#include <gtest/gtest.h>

class AVLTreeTest : public ::testing::Test
{
   protected:
    struct MockNode
    {
        int       key;
        int       height;
        MockNode* left;
        MockNode* right;

        MockNode(int k, MockNode* l = nullptr, MockNode* r = nullptr)
            : key(k), height(1), left(l), right(r)
        {
        }
    };

    AVLTree<MockNode> avl;

    MockNode* makeNode(int key, MockNode* left = nullptr, MockNode* right = nullptr)
    {
        return new MockNode(key, left, right);
    }
};

TEST_F(AVLTreeTest, HeightWorks)
{
    MockNode* n = makeNode(10);
    EXPECT_EQ(avl.height(n), 1) << "Height of a leaf node (key=" << n->key << ") should be 1.";
    delete n;
}

TEST_F(AVLTreeTest, UpdateHeightWorks)
{
    MockNode* m = makeNode(20);
    MockNode* n = makeNode(10, m);

    EXPECT_EQ(avl.height(n), 1) << "Initial height of node (key=" << n->key << ") should be 1.";
    avl.updateHeight(n);
    EXPECT_EQ(avl.height(n), 2) << "Height after adding one child to node (key=" << n->key
                                << ") should be 2.";

    delete m;
    delete n;
}

TEST_F(AVLTreeTest, BalanceFactorWorks)
{
    MockNode* left  = makeNode(10);
    MockNode* right = makeNode(5);
    MockNode* root  = makeNode(2, left, right);

    EXPECT_EQ(avl.balanceFactor(root), 0)
            << "Balance factor should be 0 when left and right subtrees have equal height.";

    root->left = nullptr;
    EXPECT_EQ(root->left, nullptr) << "Left child of root should be null after removal.";

    EXPECT_EQ(avl.balanceFactor(root), -1)
            << "Balance factor should be -1 when right subtree is taller by 1.";

    root->right = nullptr;
    root->left  = left;
    EXPECT_EQ(avl.balanceFactor(root), 1)
            << "Balance factor should be 1 when left subtree is taller by 1.";

    delete left;
    delete right;
    delete root;
}

TEST_F(AVLTreeTest, RightRotationWorks)
{
    MockNode* c = makeNode(10);
    MockNode* b = makeNode(20, c);
    MockNode* a = makeNode(30, b);

    avl.rotateRight(a);

    EXPECT_EQ(b->left, c) << "Left Child of node(key = " << b->key
                          << " ) after rotation should be node(key = " << c->key;

    EXPECT_EQ(b->right, a) << "Right Child of node(key = " << b->key
                           << " ) after rotation should be node(key = " << a->key;

    delete a;
    delete b;
    delete c;
}

TEST_F(AVLTreeTest, LeftRotationWorks)
{
    MockNode* c = makeNode(10);
    MockNode* b = makeNode(20, nullptr, c);
    MockNode* a = makeNode(30, nullptr, b);

    avl.rotateLeft(a);

    EXPECT_EQ(b->right, c) << "Right Child of node(key = " << b->key
                           << " ) after rotation should be node(key = " << c->key;

    EXPECT_EQ(b->left, a) << "Left Child of node(key = " << b->key
                          << " ) after rotation should be node(key = " << a->key;

    delete a;
    delete b;
    delete c;
}

TEST_F(AVLTreeTest, LeftRightRotationWorks)
{
    MockNode* n20 = makeNode(20);
    MockNode* n10 = makeNode(10, nullptr, n20);
    MockNode* n30 = makeNode(30, n10, nullptr);

    n30->left = avl.rotateLeft(n10);

    MockNode* newRoot = avl.rotateRight(n30);

    EXPECT_EQ(newRoot, n20) << "After LR rotation, new root should be 20.";
    EXPECT_EQ(newRoot->left, n10) << "Left child of root should be 10.";
    EXPECT_EQ(newRoot->right, n30) << "Right child of root should be 30.";

    delete n10;
    delete n20;
    delete n30;
}

TEST_F(AVLTreeTest, RightLeftRotationWorks)
{
    MockNode* n20 = makeNode(20);
    MockNode* n30 = makeNode(30, n20, nullptr);
    MockNode* n10 = makeNode(10, nullptr, n30);

    n10->right = avl.rotateRight(n30);
    MockNode* newRoot = avl.rotateLeft(n10);

    EXPECT_EQ(newRoot, n20) << "After RL rotation, new root should be 20.";
    EXPECT_EQ(newRoot->left, n10) << "Left child of root should be 10.";
    EXPECT_EQ(newRoot->right, n30) << "Right child of root should be 30.";

    delete n10;
    delete n20;
    delete n30;
}
