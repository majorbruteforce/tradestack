#include <iostream>
#include <order.hpp>
#include <price_level_node.hpp>
#include <side_tree.hpp>

int main() {
    SideTree<PriceLevelNode> st;

    Order oA("MCK458", "C245", 10, 25, Side::Buy, OrderType::Limit);
    Order oB("MCK564", "C325", 20, 16, Side::Buy, OrderType::Limit);
    Order oC("MCK154", "C226", 30, 32, Side::Buy, OrderType::Limit);
    Order oD("MCK574", "C386", 30, 32, Side::Buy, OrderType::Limit);
    Order oE("MCK964", "C455", 30, 32, Side::Buy, OrderType::Limit);

    PriceLevelNode* A = st.insert(oA);
    PriceLevelNode* B = st.insert(oB);
    PriceLevelNode* C = st.insert(oC);

    st.print();
    std::cout << std::endl;

    PriceLevelNode* D = st.insert(oD);
    PriceLevelNode* E = st.insert(oE);
    st.print();
    std::cout << std::endl;

    PriceLevelNode* F = st.remove(oA);
    st.print();
    std::cout << std::endl;

    PriceLevelNode* G = st.remove(oC);
    st.print();
    std::cout << std::endl;

    return 0;
}