#pragma once
#include <cstdint>
#include <string>
#include <utility>

#include "utils/print_utils.hpp"

enum class Side
{
    Buy,
    Sell
};
enum class OrderType
{
    Market,
    Limit
};

struct Order
{
    std::string id;
    std::string clientOrderId;

    std::uint64_t price;
    std::uint64_t remainingQuantity;
    std::uint64_t filledQuantity = 0;

    Side      side;
    OrderType type;

    explicit Order(
            std::string id, std::string cid, std::uint64_t p, std::uint64_t q, Side s, OrderType t)
        : id(std::move(id)),
          clientOrderId(std::move(cid)),
          price(p),
          remainingQuantity(q),
          side(s),
          type(t)
    {
    }
};

struct OrderRequest
{
    std::string   clientOrderId;
    std::string   symbol;
    Side          side;
    OrderType     type;
    std::uint64_t price;
    std::uint64_t quantity;
};

Order* createOrder(const OrderRequest& req);

inline void printOrder(const Order& ord, std::ostream& os = std::cout, std::size_t width = 15)
{
    utils::printField("ID", ord.id, width, os);
    utils::printField("Client ID", ord.clientOrderId, width, os);
    utils::printField("Price", ord.price, width, os);
    utils::printField("Filled Qty", ord.filledQuantity, width, os);
    utils::printField("Remaining Qty", ord.remainingQuantity, width, os);
    utils::printField("Side", (ord.side == Side::Buy ? "Buy" : "Sell"), width, os);
    utils::printField("Type", (ord.type == OrderType::Limit ? "Limit" : "Market"), width, os);
}

inline std::ostream& operator<<(std::ostream& os, const Order& o)
{
    return os << '{' << "id=" << o.id << ", cid=" << o.clientOrderId << ", price=" << o.price
              << ", rem=" << o.remainingQuantity << ", side=" << (o.side == Side::Buy ? "B" : "S")
              << ", type=" << (o.type == OrderType::Limit ? "L" : "M") << '}';
}
