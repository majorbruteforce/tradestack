#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <utils/print_utils.hpp>

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
    using Clock     = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    std::string id;
    std::string clientOrderId;

    std::uint64_t price;
    std::uint64_t remainingQuantity;
    std::uint64_t filledQuantity = 0;

    Side      side;
    OrderType type;

    TimePoint     arrivalTime;
    std::uint64_t arrivalNs;  // for persistence

    explicit Order(
            std::string id, std::string cid, std::uint64_t p, std::uint64_t q, Side s, OrderType t)
        : id(std::move(id)),
          clientOrderId(std::move(cid)),
          price(p),
          remainingQuantity(q),
          side(s),
          type(t)
    {
        setArrivalNow();
    }

    void setArrivalNow()
    {
        arrivalTime = Clock::now();
        arrivalNs   = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(arrivalTime.time_since_epoch())
                        .count());
    }

    void setArrivalFromNs(std::uint64_t ns)
    {
        arrivalNs   = ns;
        arrivalTime = TimePoint(std::chrono::nanoseconds(ns));
    }

    void setPrice(std::uint64_t p) { price = p; }
    void setRemainingQuantity(std::uint64_t q) { remainingQuantity = q; }
    void setFilledQuantity(std::uint64_t q) { filledQuantity = q; }
    void setSide(Side s) { side = s; }
    void setType(OrderType t) { type = t; }
    void setClientOrderId(std::string cid) { clientOrderId = std::move(cid); }
    void setId(std::string oid) { id = std::move(oid); }

    [[nodiscard]] const std::string& getId() const { return id; }
    [[nodiscard]] const std::string& getClientOrderId() const { return clientOrderId; }
    [[nodiscard]] std::uint64_t      getPrice() const { return price; }
    [[nodiscard]] std::uint64_t      getRemainingQuantity() const { return remainingQuantity; }
    [[nodiscard]] std::uint64_t      getFilledQuantity() const { return filledQuantity; }
    [[nodiscard]] Side               getSide() const { return side; }
    [[nodiscard]] OrderType          getType() const { return type; }
    [[nodiscard]] TimePoint          getArrivalTime() const { return arrivalTime; }
    [[nodiscard]] std::uint64_t      getArrivalNs() const { return arrivalNs; }
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
    utils::printField("ArrivalNs", ord.getArrivalNs(), width, os);
}

inline std::ostream& operator<<(std::ostream& os, const Order& o)
{
    return os << '{' << "id=" << o.id << ", cid=" << o.clientOrderId << ", price=" << o.price
              << ", rem=" << o.remainingQuantity << ", side=" << (o.side == Side::Buy ? "B" : "S")
              << ", type=" << (o.type == OrderType::Limit ? "L" : "M")
              << ", arrivalNs=" << o.getArrivalNs() << '}';
}
