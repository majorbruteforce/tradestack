#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include "utils/print_utils.hpp"

/**
 * @file Order.hpp
 * @brief Defines trading order types, structures, and utility functions.
 *
 * This header contains the definitions for `Order`, `OrderRequest`, and related
 * functions used for representing and managing financial orders in a trading system.
 */

namespace chrono = std::chrono;

namespace tradestack {
    /**
     * @brief Side of the order (buy or sell).
     */
    enum class Side : uint8_t {
        Buy,  ///< Buy order
        Sell  ///< Sell order
    };

    /**
     * @brief Type of the order (market or limit).
     */
    enum class OrderType : uint8_t {
        Market,  ///< Market order: executed immediately at best available price
        Limit  ///< Limit order: executed only at specified or better price
    };

    /// Type alias for representing price.
    using price_type = uint64_t;
    /// Type alias for representing order quantity.
    using quantity_type = uint64_t;
    /// Type alias for unique order identifier.
    using order_id_type = std::string;
    /// Type alias for client-side order identifier.
    using client_id_type = std::string;

    /**
     * @brief Represents a single order in the trading system.
     *
     * Stores all relevant metadata including IDs, price, quantities, side,
     * type, and arrival timestamp. Provides getters and setters for easy
     * modification and retrieval.
     */
    struct Order {
        using Clock = chrono::system_clock;  ///< Clock type used for arrival time
        using TimePoint = chrono::time_point<Clock>;  ///< Time point representing arrival time
        using iterator =
            std::list<Order>::iterator;  ///< Iterator for order placement in price levels

        order_id_type id;  ///< Unique order identifier
        client_id_type clientOrderId;  ///< Client-provided order identifier

        price_type price{};  ///< Limit price of the order
        quantity_type remainingQuantity{};  ///< Remaining unfilled quantity
        quantity_type filledQuantity{0};  ///< Quantity already filled

        Side side{};  ///< Buy or Sell
        OrderType type{};  ///< Market or Limit

        TimePoint arrivalTime;  ///< System timestamp of order arrival
        uint64_t arrivalNs{};  ///< Arrival time in nanoseconds since epoch (for persistence)
        iterator level_pos;  ///< Position in price level list (non-owning)

        /**
         * @brief Construct a new Order object.
         *
         * @param oid Unique order ID.
         * @param cid Client order ID.
         * @param p Price of the order.
         * @param q Quantity of the order.
         * @param s Side (buy/sell).
         * @param t Order type (market/limit).
         */
        Order(order_id_type oid, client_id_type cid, price_type p, quantity_type q, Side s,
              OrderType t) :
            id(std::move(oid)), clientOrderId(std::move(cid)), price(p), remainingQuantity(q),
            side(s), type(t) {
            setArrivalNow();
        }

        /**
         * @brief Set the arrival timestamp to the current system time.
         */
        void setArrivalNow() {
            arrivalTime = Clock::now();
            arrivalNs = static_cast<std::uint64_t>(
                chrono::duration_cast<chrono::nanoseconds>(arrivalTime.time_since_epoch()).count());
        }

        /**
         * @brief Set arrival timestamp from nanoseconds since epoch.
         * @param ns Nanoseconds since epoch.
         */
        void setArrivalFromNs(uint64_t ns) {
            arrivalNs = ns;
            arrivalTime = TimePoint(chrono::nanoseconds(ns));
        }

        /// Set the order price.
        void setPrice(price_type p) noexcept { price = p; }

        /// Set the remaining quantity.
        void setRemainingQuantity(quantity_type q) noexcept { remainingQuantity = q; }

        /// Set the filled quantity.
        void setFilledQuantity(quantity_type q) noexcept { filledQuantity = q; }

        /// Set the order side (buy/sell).
        void setSide(Side s) noexcept { side = s; }

        /// Set the order type (market/limit).
        void setType(OrderType t) noexcept { type = t; }

        /// Set client order ID.
        void setClientOrderId(client_id_type cid) { clientOrderId = std::move(cid); }

        /// Set system order ID.
        void setId(order_id_type oid) { id = std::move(oid); }

        /// @return Order ID as string view.
        [[nodiscard]] std::string_view getId() const noexcept { return id; }

        /// @return Client order ID as string view.
        [[nodiscard]] std::string_view getClientOrderId() const noexcept { return clientOrderId; }

        /// @return Order price.
        [[nodiscard]] price_type getPrice() const noexcept { return price; }

        /// @return Remaining unfilled quantity.
        [[nodiscard]] quantity_type getRemainingQuantity() const noexcept {
            return remainingQuantity;
        }

        /// @return Filled quantity.
        [[nodiscard]] quantity_type getFilledQuantity() const noexcept { return filledQuantity; }

        /// @return Order side.
        [[nodiscard]] Side getSide() const noexcept { return side; }

        /// @return Order type.
        [[nodiscard]] OrderType getType() const noexcept { return type; }

        /// @return Arrival timestamp as chrono::time_point.
        [[nodiscard]] TimePoint getArrivalTime() const noexcept { return arrivalTime; }

        /// @return Arrival timestamp as nanoseconds since epoch.
        [[nodiscard]] std::uint64_t getArrivalNs() const noexcept { return arrivalNs; }
    };

    /**
     * @brief Request object used for creating a new order.
     *
     * Encapsulates client-provided details before creating an `Order`.
     */
    struct OrderRequest {
        client_id_type clientOrderId;  ///< Client order ID
        std::string symbol;  ///< Trading symbol (e.g., "AAPL", "BTC-USD")
        Side side;  ///< Buy or Sell
        OrderType type;  ///< Market or Limit
        price_type price;  ///< Price (used for limit orders)
        quantity_type quantity;  ///< Total order quantity
    };

    /**
     * @brief Create a new Order object from an OrderRequest.
     *
     * @param req Order request containing client-provided details.
     * @return Unique pointer to a newly created Order.
     */
    std::unique_ptr<Order> createOrder(const OrderRequest& req);

    /**
     * @brief Pretty-print an Order to an output stream.
     *
     * Prints order details (ID, client ID, price, quantities, side, type, arrival time).
     *
     * @param ord Order to print.
     * @param os Output stream (default: std::cout).
     * @param width Field width for formatting (default: 15).
     */
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

    /**
     * @brief Stream operator for Order.
     *
     * Provides a compact string representation for debugging/logging.
     * Example:
     * ```
     * {id=123, cid=ABC, price=100, rem=50, side=B, type=L, arrivalNs=123456789}
     * ```
     *
     * @param os Output stream.
     * @param o Order to print.
     * @return Reference to the output stream.
     */
    inline std::ostream& operator<<(std::ostream& os, const Order& o) {
        return os << '{' << "id=" << o.id << ", cid=" << o.clientOrderId << ", price=" << o.price
                  << ", rem=" << o.remainingQuantity
                  << ", side=" << (o.side == Side::Buy ? "B" : "S")
                  << ", type=" << (o.type == OrderType::Limit ? "L" : "M")
                  << ", arrivalNs=" << o.getArrivalNs() << '}';
    }
}  // namespace tradestack
