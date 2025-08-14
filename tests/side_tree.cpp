#include <gtest/gtest.h>
#include <order.hpp>
#include <side_tree.hpp>

class SideTreeTest : public ::testing::Test {
   protected:
    struct MockNode {
        int                price;
        int                height;
        std::deque<Order*> level;
        MockNode*          left;
        MockNode*          right;

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

    ASSERT_NE(first, nullptr) << "z returned nullptr for first node";
    ASSERT_NE(second, nullptr) << "Insert returned nullptr for second node";

    EXPECT_EQ(st->root, first) << "Root node should be node(price = " << first->price << ")";
    EXPECT_EQ(st->root->right, second)
            << "Right of root node should be node(price = " << second->price << ")";
    EXPECT_EQ(st->root->left, nullptr) << "Left of root node should be nullptr";

    MockNode* third = st->insert(o30);

    ASSERT_NE(third, nullptr) << "Insert returned nullptr for third node";

    EXPECT_EQ(st->root, second) << "Root node should be node(price = " << second->price << ")";
    EXPECT_EQ(st->root->right, third)
            << "Right of root node should be node(price = " << third->price << ")";
    EXPECT_EQ(st->root->left, first)
            << "Right of root node should be node(price = " << first->price << ")";
}

TEST_F(SideTreeTest, FindsPriceLevel) {
}