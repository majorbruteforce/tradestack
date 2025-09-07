#include <print>
#include <vector>

#include "order.hpp"
#include "side_tree.hpp"

using namespace tradestack;

int main() {
    SideTree st{Side::Buy};

    std::vector<Order> orders{
        {"MCK458", "C245", 10, 25, Side::Buy, OrderType::Limit},
        {"MCK564", "C325", 20, 16, Side::Buy, OrderType::Limit},
        {"MCK154", "C226", 30, 32, Side::Buy, OrderType::Limit},
        {"MCK574", "C386", 30, 32, Side::Buy, OrderType::Limit},
        {"MCK964", "C455", 30, 32, Side::Buy, OrderType::Limit},
    };

    for (int i = 0; i < 3; ++i) {
        st.insert(orders[i]);
    }

    st.print();
    std::print("\n");
    for (int i = 3; i < std::ssize(orders); ++i) {
        st.insert(orders[i]);
    }

    st.print();
    std::print("\n");

    st.remove(orders[0]);
    st.print();
    std::print("\n");

    st.remove(orders[2]);
    st.print();
    std::print("\n");

    if (auto best = st.top(); !best.empty()) {
        std::println("Best order: {} at price {}", best.front()->id, best.front()->price);
    }

    return 0;
}
