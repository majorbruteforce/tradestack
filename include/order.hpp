#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include "utils/print_utils.hpp"

namespace chrono = std::chrono;

namespace tradestack {
    enum class Side : uint8_t { Buy, Sell };
    enum class OrderType : uint8_t { Market, Limit };

    using price_type = uint64_t;
    using quantity_type = uint64_t;
    using order_id_type = std::string;
    using client_id_type = std::string;

    struct Order {
        using Clock = chrono::system_clock;
        using TimePoint = chrono::time_point<Clock>;
        using iterator = std::list<Order>::iterator;

        order_id_type id;
        client_id_type clientOrderId;

        price_type price{};
        quantity_type remainingQuantity{};
        quantity_type filledQuantity{0};

        Side side{};
        OrderType type{};

        TimePoint arrivalTime;
        std::uint64_t arrivalNs{};  // for persistence
        iterator level_pos;

        Order(order_id_type oid, client_id_type cid, price_type p, quantity_type q, Side s,
              OrderType t) :
            id(std::move(oid)), clientOrderId(std::move(cid)), price(p), remainingQuantity(q),
            side(s), type(t) {
            setArrivalNow();
        }

        void setArrivalNow() {
            arrivalTime = Clock::now();
            arrivalNs = static_cast<std::uint64_t>(
                chrono::duration_cast<chrono::nanoseconds>(arrivalTime.time_since_epoch()).count());
        }

        void setArrivalFromNs(std::uint64_t ns) {
            arrivalNs = ns;
            arrivalTime = TimePoint(chrono::nanoseconds(ns));
        }

        void setPrice(price_type p) noexcept { price = p; }
        void setRemainingQuantity(quantity_type q) noexcept { remainingQuantity = q; }
        void setFilledQuantity(quantity_type q) noexcept { filledQuantity = q; }
        void setSide(Side s) noexcept { side = s; }
        void setType(OrderType t) noexcept { type = t; }
        void setClientOrderId(client_id_type cid) { clientOrderId = std::move(cid); }
        void setId(order_id_type oid) { id = std::move(oid); }

        [[nodiscard]] std::string_view getId() const noexcept { return id; }
        [[nodiscard]] std::string_view getClientOrderId() const noexcept { return clientOrderId; }
        [[nodiscard]] price_type getPrice() const noexcept { return price; }
        [[nodiscard]] quantity_type getRemainingQuantity() const noexcept {
            return remainingQuantity;
        }

        [[nodiscard]] quantity_type getFilledQuantity() const noexcept { return filledQuantity; }
        [[nodiscard]] Side getSide() const noexcept { return side; }
        [[nodiscard]] OrderType getType() const noexcept { return type; }
        [[nodiscard]] TimePoint getArrivalTime() const noexcept { return arrivalTime; }
        [[nodiscard]] std::uint64_t getArrivalNs() const noexcept { return arrivalNs; }
    };

    struct OrderRequest {
        client_id_type clientOrderId;
        std::string symbol;
        Side side;
        OrderType type;
        price_type price;
        quantity_type quantity;
    };

    std::unique_ptr<Order> createOrder(const OrderRequest& req);

    inline void printOrder(const Order& ord, std::ostream& os = std::cout, std::size_t width = 15) {
        utils::printField("ID", ord.id, width, os);
        utils::printField("Client ID", ord.clientOrderId, width, os);
        utils::printField("Price", ord.price, width, os);
        utils::printField("Filled Qty", ord.filledQuantity, width, os);
        utils::printField("Remaining Qty", ord.remainingQuantity, width, os);
        utils::printField("Side", ord.side == Side::Buy ? "Buy" : "Sell", width, os);
        utils::printField("Type", ord.type == OrderType::Limit ? "Limit" : "Market", width, os);
        utils::printField("ArrivalNs", ord.getArrivalNs(), width, os);
    }

    inline std::ostream& operator<<(std::ostream& os, const Order& o) {
        return os << '{' << "id=" << o.id << ", cid=" << o.clientOrderId << ", price=" << o.price
                  << ", rem=" << o.remainingQuantity
                  << ", side=" << (o.side == Side::Buy ? "B" : "S")
                  << ", type=" << (o.type == OrderType::Limit ? "L" : "M")
                  << ", arrivalNs=" << o.getArrivalNs() << '}';
    }
}  // namespace tradestack
