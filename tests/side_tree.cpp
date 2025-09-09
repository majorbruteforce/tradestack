#include "side_tree.hpp"

#include <gtest/gtest.h>

#include "order.hpp"

using namespace tradestack;

class SideTreeTest : public ::testing::Test {
protected:
    SideTree* st;

    void SetUp() override { st = new SideTree(Side::Buy); }
    void TearDown() override { delete st; }
};

TEST_F(SideTreeTest, InsertsInEmptyTree) {
    EXPECT_TRUE(st->empty()) << "Tree should start empty";

    Order o("MCK123", "C456", 100, 10, Side::Buy, OrderType::Limit);
    auto* node = st->insert(o);

    EXPECT_FALSE(st->empty()) << "Tree must not be empty after an insert";
    EXPECT_EQ(st->size(), 1);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->price, 100);
    EXPECT_EQ(node->level.size(), 1);
}

TEST_F(SideTreeTest, InsertsMultiplePrices) {
    Order o10("MCK458", "C245", 10, 25, Side::Buy, OrderType::Limit);
    Order o20("MCK564", "C325", 20, 16, Side::Buy, OrderType::Limit);
    Order o30("MCK154", "C426", 30, 32, Side::Buy, OrderType::Limit);

    auto* n10 = st->insert(o10);
    auto* n20 = st->insert(o20);
    auto* n30 = st->insert(o30);

    ASSERT_NE(n10, nullptr);
    ASSERT_NE(n20, nullptr);
    ASSERT_NE(n30, nullptr);

    EXPECT_EQ(st->low()->price, 10);
    EXPECT_EQ(st->high()->price, 30);
    EXPECT_EQ(st->size(), 3);
}

TEST_F(SideTreeTest, InsertsSamePriceAggregates) {
    Order oA("MCK154", "C426", 30, 32, Side::Buy, OrderType::Limit);
    Order oB("MCK574", "C386", 30, 32, Side::Buy, OrderType::Limit);
    Order oC("MCK964", "C455", 30, 32, Side::Buy, OrderType::Limit);

    auto* n1 = st->insert(oA);
    auto* n2 = st->insert(oB);
    auto* n3 = st->insert(oC);

    EXPECT_EQ(n1, n2);
    EXPECT_EQ(n1, n3);
    EXPECT_EQ(n1->level.size(), 3);
}

TEST_F(SideTreeTest, RemovesOrder) {
    Order o10("MCK458", "C245", 10, 25, Side::Buy, OrderType::Limit);
    Order o20("MCK564", "C325", 20, 16, Side::Buy, OrderType::Limit);

    auto* n10 = st->insert(o10);
    auto* n20 = st->insert(o20);

    EXPECT_EQ(st->size(), 2);

    auto* res = st->remove(o10);
    EXPECT_EQ(res, nullptr) << "Node at price=10 should be erased completely";
    EXPECT_EQ(st->size(), 1);

    res = st->remove(o20);
    EXPECT_EQ(res, nullptr);
    EXPECT_TRUE(st->empty());
}

TEST_F(SideTreeTest, FindsPriceLevel) {
    Order o10("MCK458", "C245", 10, 25, Side::Buy, OrderType::Limit);
    Order o20("MCK564", "C325", 20, 16, Side::Buy, OrderType::Limit);

    st->insert(o10);
    st->insert(o20);

    auto* n10 = st->find(10);
    auto* n20 = st->find(20);
    auto* n30 = st->find(30);

    ASSERT_NE(n10, nullptr);
    EXPECT_EQ(n10->price, 10);

    ASSERT_NE(n20, nullptr);
    EXPECT_EQ(n20->price, 20);

    EXPECT_EQ(n30, nullptr);
}

TEST_F(SideTreeTest, TopOrdersRespectsSide) {
    Order o10("MCK458", "C245", 10, 25, Side::Buy, OrderType::Limit);
    Order o20("MCK564", "C325", 20, 16, Side::Buy, OrderType::Limit);
    Order o30("MCK154", "C426", 30, 32, Side::Buy, OrderType::Limit);

    st->insert(o10);
    st->insert(o20);
    st->insert(o30);

    auto topOrders = st->top(2);
    ASSERT_EQ(topOrders.size(), 2);
    EXPECT_EQ(topOrders[0]->price, 30);
    EXPECT_EQ(topOrders[1]->price, 20);

    st->setSide(Side::Sell);
    auto sellTop = st->top(2);
    ASSERT_EQ(sellTop.size(), 2);
    EXPECT_EQ(sellTop[0]->price, 10);
    EXPECT_EQ(sellTop[1]->price, 20);
}
