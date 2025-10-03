#pragma once
#include <string>

#include "order.hpp"
#include "price_level_node.hpp"
#include "side_tree.hpp"
#include "utils/string.hpp"
class Instrument {
   public:
   public:
    Instrument() noexcept = default;

    explicit Instrument(const std::string &sym) noexcept
        : symbol(sym), last_trade_ts(std::chrono::system_clock::time_point{}) {}

    const std::string &getSymbol() const noexcept { return symbol; }

    const SideTree<PriceLevelNode> &getBuySide() const noexcept { return buy_side; }
    const SideTree<PriceLevelNode> &getSellSide() const noexcept { return sell_side; }

    const std::unordered_map<OrderId, Order *> &getOrderMap() const noexcept { return order_map; }

    Order *findOrder(const OrderId &id) const noexcept {
        auto it = order_map.find(id);
        return it == order_map.end() ? nullptr : it->second;
    }

    double      getLastTradePrice() const noexcept { return last_trade_price; }
    uint64_t    getLastTradeSize() const noexcept { return last_trade_size; }
    std::string getLastTradeTimestamp() const noexcept {
        return timepoint_to_string(last_trade_ts);
    }

    uint64_t getVolumeToday() const noexcept { return volume_today; }
    double   getVWAPNumerator() const noexcept { return vwap_numerator; }

    double getVWAP() const noexcept {
        return (volume_today == 0) ? 0.0 : (vwap_numerator / static_cast<double>(volume_today));
    }

    double getOpen() const noexcept { return open; }
    double getHigh() const noexcept { return high; }
    double getLow() const noexcept { return low; }
    double getClose() const noexcept { return close; }

    void updateState(double fillPrice, int qty);
    void fetchState(std::string clientId);

    std::vector<Order *> getClientOrders(std::string clientId) {
        return (client_orders.find(clientId) != client_orders.end()) ? client_orders[clientId]
                                                                     : std::vector<Order *>();
    }

    void placeOrder(Order &order);
    void execute_limit_if_match();
    void execute_market();

   private:
    std::string              symbol;
    SideTree<PriceLevelNode> buy_side;
    SideTree<PriceLevelNode> sell_side;

    std::unordered_map<OrderId, Order *> order_map;

    std::unordered_map<std::string, std::vector<Order *>> client_orders;

    double                                last_trade_price{0.0};
    uint64_t                              last_trade_size{0};
    std::chrono::system_clock::time_point last_trade_ts;
    uint64_t                              volume_today{0};
    double                                vwap_numerator{0.0};
    double                                open{0.0}, high{0.0}, low{0.0}, close{0.0};
};