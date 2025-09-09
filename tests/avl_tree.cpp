#include "avl_tree.hpp"

#include <cstdint>
#include <gtest/gtest.h>
#include <vector>

using tradestack::AVLTree;

class AVLTreeTest : public ::testing::Test {
protected:
    AVLTree<int> avl;
};

TEST_F(AVLTreeTest, InsertAndInorderAreSortedUnique) {
    auto [n30, c30a] = avl.insert(30);
    auto [n10, c10] = avl.insert(10);
    auto [n50, c50] = avl.insert(50);
    auto [n20, c20] = avl.insert(20);
    auto [n25, c25] = avl.insert(25);
    auto [n30b, c30b] = avl.insert(30);

    EXPECT_TRUE(c30a);
    EXPECT_TRUE(c10);
    EXPECT_TRUE(c50);
    EXPECT_TRUE(c20);
    EXPECT_TRUE(c25);
    EXPECT_FALSE(c30b);
    EXPECT_EQ(n30, n30b);

    std::vector<int> keys;
    avl.inorder([&](auto* node) { keys.push_back(node->key); });
    std::vector<int> expected{10, 20, 25, 30, 50};
    EXPECT_EQ(keys, expected);
}

TEST_F(AVLTreeTest, FindMinMaxWork) {
    (void) avl.insert(40);
    (void) avl.insert(10);
    (void) avl.insert(70);
    (void) avl.insert(25);

    auto mn = avl.findMin();
    auto mx = avl.findMax();
    ASSERT_NE(mn, nullptr);
    ASSERT_NE(mx, nullptr);
    EXPECT_EQ(mn->key, 10);
    EXPECT_EQ(mx->key, 70);
}

TEST_F(AVLTreeTest, FindWorks) {
    (void) avl.insert(5);
    (void) avl.insert(2);
    (void) avl.insert(9);

    auto n2 = avl.find(2);
    auto n9 = avl.find(9);
    auto n7 = avl.find(7);

    ASSERT_NE(n2, nullptr);
    ASSERT_NE(n9, nullptr);
    EXPECT_EQ(n2->key, 2);
    EXPECT_EQ(n9->key, 9);
    EXPECT_EQ(n7, nullptr);
}

TEST_F(AVLTreeTest, EraseWorksAndUpdatesMinMax) {
    (void) avl.insert(10);
    (void) avl.insert(20);
    (void) avl.insert(30);

    ASSERT_NE(avl.findMin(), nullptr);
    ASSERT_NE(avl.findMax(), nullptr);
    EXPECT_EQ(avl.findMin()->key, 10);
    EXPECT_EQ(avl.findMax()->key, 30);

    EXPECT_TRUE(avl.erase(20));
    EXPECT_EQ(avl.find(20), nullptr);

    EXPECT_TRUE(avl.erase(10));
    ASSERT_NE(avl.findMin(), nullptr);
    EXPECT_EQ(avl.findMin()->key, 30);

    EXPECT_TRUE(avl.erase(30));
    EXPECT_EQ(avl.findMin(), nullptr);
    EXPECT_EQ(avl.findMax(), nullptr);

    EXPECT_FALSE(avl.erase(1234));
}

TEST_F(AVLTreeTest, InorderLimitStopsEarly) {
    for (int k: {40, 10, 70, 25, 5, 60}) {
        (void) avl.insert(k);
    }

    std::vector<int> first3;
    avl.inorder([&](auto* n) { first3.push_back(n->key); }, 3);
    // Sorted order is {5, 10, 25, 40, 60, 70}; first 3:
    std::vector<int> expected{5, 10, 25};
    EXPECT_EQ(first3, expected);
}

TEST(AVLTreeStandalone, HeterogeneousLookupWithTransparentLess) {
    AVLTree<uint64_t, std::less<>> tree;
    (void) tree.insert(100u);
    (void) tree.insert(200u);

    unsigned long long probe = 100ULL;
    auto n = tree.find(probe);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->key, 100u);

    EXPECT_TRUE(tree.erase(200ULL));
    EXPECT_EQ(tree.find(200u), nullptr);
}
