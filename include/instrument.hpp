#include <string>

#include "order.hpp"
#include "price_level_node.hpp"
#include "side_tree.hpp"

class Insturment {
   private:
    std::string              symbol;
    SideTree<PriceLevelNode> buy_side;
    SideTree<PriceLevelNode> sell_side;

    std::unordered_map<OrderId, Order*> order_map;

    double                                last_trade_price{0.0};
    uint64_t                              last_trade_size{0};
    std::chrono::system_clock::time_point last_trade_ts;
    uint64_t                              volume_today{0};
    double                                vwap_numerator{0.0};
    double                                open{0.0}, high{0.0}, low{0.0}, close{0.0};
};