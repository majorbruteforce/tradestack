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

        void leanCopy(const MockNode* other) {
            if (!other)
                return;
            price  = other->price;
            level  = other->level;
            height = other->height;
        }

        MockNode(int k, MockNode* l = nullptr, MockNode* r = nullptr)
            : price(k), height(1), left(l), right(r) {}
    };
    Order *o, *o10, *o20, *o30;

    SideTree<MockNode>* st;

    void SetUp() override {
        st  = new SideTree<MockNode>();
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

    EXPECT_EQ(st->low->price, 10) << "Highest price level should be 10";
    EXPECT_EQ(st->high->price, 30) << "Lowest price level should be 30";
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

    ASSERT_NE(A, nullptr);
    ASSERT_NE(B, nullptr);
    ASSERT_NE(C, nullptr);

    EXPECT_EQ(A->level.size(), 1);
    EXPECT_EQ(A->level.front()->getId(), "MCK458");
    EXPECT_EQ(B->level.size(), 1);
    EXPECT_EQ(B->level.front()->getId(), "MCK564");
    EXPECT_EQ(C->level.size(), 1);
    EXPECT_EQ(C->level.front()->getId(), "MCK154");

    MockNode* D = st->insert(oD);
    MockNode* E = st->insert(oE);

    ASSERT_NE(D, nullptr);
    ASSERT_NE(E, nullptr);

    EXPECT_EQ(D, C);
    EXPECT_EQ(E, C);

    EXPECT_EQ(C->level.size(), 3);
    EXPECT_EQ(C->level.front()->getId(), "MCK154");
    EXPECT_EQ(C->level.back()->getId(), "MCK964");

    MockNode* F = st->remove(oA);
    EXPECT_EQ(F, nullptr) << "Removing last order at price 10 must delete node";

    MockNode* G = st->remove(oC);

    EXPECT_EQ(G, C);
    EXPECT_EQ(C->level.size(), 2);
    EXPECT_EQ(C->level.front()->getId(), "MCK574");
    EXPECT_EQ(C->level.back()->getId(), "MCK964");
}


TEST_F(SideTreeTest, FindsPriceLevel) {
}