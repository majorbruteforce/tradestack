#include <gtest/gtest.h>
#include <order.hpp>
#include <side_tree.hpp>

class SideTreeTest : public ::testing::Test {
   protected:
    struct MockNode {
        int               price;
        int               height;
        std::list<Order*> level;
        MockNode*         left;
        MockNode*         right;

        MockNode(int k, MockNode* l = nullptr, MockNode* r = nullptr)
            : price(k), height(1), left(l), right(r) {}
    };
    Order *o, *o10, *o20, *o30;

    SideTree<MockNode>* st;

    void SetUp() override {
        st  = new SideTree<MockNode>([](const int& a, const int& b) { return a < b; });
        o   = new Order("MCK123", "C456", 100, 10, Side::Buy, OrderType::Limit);
        o10 = new Order("MCK458", "C245", 10, 25, Side::Buy, OrderType::Limit);
        o20 = new Order("MCK564", "C325", 20, 16, Side::Buy, OrderType::Limit);
        o30 = new Order("MCK154", "C426", 30, 32, Side::Buy, OrderType::Limit);
    }

    void TearDown() override {
        delete st;
        st = nullptr;

        delete o;
    }
};

TEST_F(SideTreeTest, InsertsInEmptyTree) {
    EXPECT_EQ(st->root, nullptr) << "Empty tree must have nullptr as root";

    st->insert(*o);

    EXPECT_FALSE(st->empty()) << "empty() must return false after an insertion";
    EXPECT_EQ(st->size(), 1) << "size() must return 1 after single insertion";
    EXPECT_NE(st->root, nullptr) << "root of the tree should not be nullptr after insertion";
}

TEST_F(SideTreeTest, InsertsInNonEmptyTree) {
    Order o10("MCK458", "C245", 10, 25, Side::Buy, OrderType::Limit);
    Order o20("MCK564", "C325", 20, 16, Side::Buy, OrderType::Limit);
    Order o30("MCK154", "C426", 30, 32, Side::Buy, OrderType::Limit);

    MockNode* first  = st->insert(o10);
    MockNode* second = st->insert(o20);

    ASSERT_NE(first, nullptr) << "insert() returned nullptr for first node";
    ASSERT_NE(second, nullptr) << "insert() returned nullptr for second node";

    EXPECT_EQ(st->root, first) << "Root node should be node(price = " << first->price << ")";
    EXPECT_EQ(st->root->right, second)
            << "Right of root node should be node(price = " << second->price << ")";
    EXPECT_EQ(st->root->left, nullptr) << "Left of root node should be nullptr";

    MockNode* third = st->insert(o30);

    ASSERT_NE(third, nullptr) << "insert() returned nullptr for third node";

    EXPECT_EQ(st->root, second) << "Root node should be node(price = " << second->price << ")";
    EXPECT_EQ(st->root->right, third)
            << "Right of root node should be node(price = " << third->price << ")";
    EXPECT_EQ(st->root->left, first)
            << "Right of root node should be node(price = " << first->price << ")";
}

TEST_F(SideTreeTest, RemovesOrderFromLevel) {
    Order oA("MCK458", "C245", 10, 25, Side::Buy, OrderType::Limit);
    Order oB("MCK564", "C325", 20, 16, Side::Buy, OrderType::Limit);
    Order oC("MCK154", "C226", 30, 32, Side::Buy, OrderType::Limit);
    Order oD("MCK574", "C386", 30, 32, Side::Buy, OrderType::Limit);
    Order oE("MCK964", "C455", 30, 32, Side::Buy, OrderType::Limit);

    MockNode* A = st->insert(oA);
    MockNode* B = st->insert(oB);
    MockNode* C = st->insert(oC);

    ASSERT_NE(A, nullptr) << "insert() returned nullptr for A";
    ASSERT_NE(B, nullptr) << "insert() returned nullptr for B";
    ASSERT_NE(C, nullptr) << "insert() returned nullptr for C";

    EXPECT_EQ(A->level.size(), 1) << "size of level 10 must be 1";
    EXPECT_EQ(A->level.front()->getId(), "MCK458")
            << "front of level 10 must contain order(id=MCK458)";
    EXPECT_EQ(B->level.size(), 1) << "size of level 20 must be 1";
    EXPECT_EQ(B->level.front()->getId(), "MCK564")
            << "front of level 20 must contain order(id=MCK564)";
    EXPECT_EQ(C->level.size(), 1) << "size of level 30 must be 1";
    EXPECT_EQ(C->level.front()->getId(), "MCK154")
            << "front of level 30 must contain order(id=MCK154)";

    MockNode* D = st->insert(oD);
    MockNode* E = st->insert(oE);

    ASSERT_NE(D, nullptr) << "insert() returned nullptr for D";
    ASSERT_NE(E, nullptr) << "insert() returned nullptr for E";

    EXPECT_EQ(D, C) << "insert() MCK574 must return price level 30";
    EXPECT_EQ(E, C) << "insert() MCK964 must return price level 30";

    EXPECT_EQ(A->level.size(), 1) << "size of level 10 must be 1";
    EXPECT_EQ(A->level.front()->getId(), "MCK458")
            << "front of level 10 must contain order(id=MCK458)";
    EXPECT_EQ(B->level.size(), 1) << "size of level 20 must be 1";
    EXPECT_EQ(B->level.front()->getId(), "MCK564")
            << "front of level 20 must contain order(id=MCK564)";
    EXPECT_EQ(C->level.size(), 3) << "size of level 30 must be 3";
    EXPECT_EQ(C->level.front()->getId(), "MCK154")
            << "front of level 30 must contain order(id=MCK154)";
    EXPECT_EQ(C->level.back()->getId(), "MCK964")
            << "front of level 30 must contain order(id=MCK154)";

    MockNode* F = st->remove(oA);

    EXPECT_EQ(F, A) << "price level returned for remove() of MCK458 must be 10";
    EXPECT_TRUE(A->level.empty()) << "price level 10 must be empty";

    MockNode* G = st->remove(oC);

    EXPECT_EQ(G, C) << "price level returned for remove() of MCK154 must be 30";
    EXPECT_EQ(C->level.size(), 2) << "size of level 30 must be 2";
    EXPECT_EQ(C->level.front()->getId(), "MCK574")
            << "front of level 30 must contain order(id=MCK154)";
    EXPECT_EQ(C->level.back()->getId(), "MCK964")
            << "front of level 30 must contain order(id=MCK154)";
}

TEST_F(SideTreeTest, FindsPriceLevel) {
}