#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "avl_tree.hpp"
#include "order.hpp"
#include "price_level_node.hpp"

/**
 * @file side_tree.hpp
 * @brief Defines the SideTree class for managing one side of an order book.
 *
 * A SideTree manages all price levels and orders for either the Buy or Sell side
 * of an order book. It uses an AVL tree to keep price levels ordered and a hash
 * map for direct access to price level nodes.
 */

namespace tradestack {
    /**
     * @brief Manages orders for one side (Buy or Sell) of an order book.
     *
     * `SideTree` maintains:
     * - An AVL tree (`AVLTree`) for sorted price levels.
     * - A hash map (`levels_map`) mapping prices to `PriceLevelNode`.
     * - References to the lowest and highest active price levels.
     * - The total count of active orders.
     *
     * This structure allows:
     * - O(log n) insertion/removal of price levels
     * - O(1) lookup of existing price levels
     * - Efficient retrieval of best bid/ask or top N levels
     */
    class SideTree {
    public:
        /// Price type (64-bit unsigned integer).
        using price_type = uint64_t;
        /// Alias for an individual order.
        using order_type = Order;
        /// Alias for a price level node.
        using node_type = PriceLevelNode;
        /// Raw pointer to a price level node.
        using node_ref = node_type*;
        /// Container type holding multiple orders.
        using order_list = std::vector<order_type>;
        /// Container of pointers to orders (used for top-of-book queries).
        using order_ptr_vec = std::vector<order_type*>;

        /// AVL tree type for storing price keys.
        using tree_type = AVLTree<price_type>;
        /// Node type of the AVL tree.
        using tree_node = typename tree_type::node_type;
        /// Raw pointer reference to an AVL tree node.
        using tree_node_ptr = typename tree_type::node_ref;
        /// Const raw pointer reference to an AVL tree node.
        using const_tree_node = typename tree_type::const_node;

        /// Hash map mapping price levels to nodes.
        using levels_map = std::unordered_map<price_type, node_type, std::hash<price_type>,
                                              std::equal_to<price_type>>;

        /// Iterator type for traversing price levels.
        using levels_iterator = typename levels_map::iterator;
        /// Const iterator type for traversing price levels.
        using const_levels_iterator = typename levels_map::const_iterator;

        /**
         * @brief Construct a SideTree for a given side.
         * @param side Side of the book (Buy or Sell). Defaults to Buy.
         */
        explicit SideTree(Side side = Side::Buy) noexcept : m_side{side} {}

        /// Non-copyable.
        SideTree(const SideTree&) = delete;
        SideTree& operator=(const SideTree&) = delete;

        /// Movable.
        SideTree(SideTree&&) noexcept = default;
        SideTree& operator=(SideTree&&) noexcept = default;

        /// Destructor.
        ~SideTree() = default;

        /**
         * @brief Check whether the tree has no orders.
         * @return true if empty, false otherwise.
         */
        [[nodiscard]] bool empty() const noexcept { return m_orderCnt == 0; }

        /**
         * @brief Get the number of orders stored in this side.
         * @return Count of active orders.
         */
        [[nodiscard]] size_t size() const noexcept { return m_orderCnt; }

        /**
         * @brief Insert an order into the tree.
         *
         * - If the price level does not exist, creates a new one.
         * - Inserts the order at the back of the level’s order list.
         *
         * @param order Order to insert.
         * @return Pointer to the price level node where the order was inserted.
         */
        node_ref insert(const Order& order) {
            const price_type px = order.price;
            if (!m_levels.contains(px)) {
                (void) m_avl.insert(px);
                auto& node = m_levels[px];
                node.price = px;
                node.height = 1;
                touchRangeOnInsert(&node);
            }

            auto& levelNode = m_levels.at(px);
            levelNode.level.push_back(order);
            ++m_orderCnt;
            return &levelNode;
        }

        /**
         * @brief Remove an order from the tree.
         *
         * - Matches by `id` and optionally `clientOrderId`.
         * - If the level becomes empty, removes the level from both the AVL tree and hash map.
         *
         * @param order Order to remove.
         * @return Pointer to the level node after removal, or nullptr if the level was erased.
         */
        node_ref remove(const Order& order) {
            const price_type px = order.price;
            auto it = m_levels.find(px);
            if (it == m_levels.end()) {
                return nullptr;
            }

            auto& level = it->second.level;
            auto match = [&](const Order& o) {
                if (order.clientOrderId.empty()) {
                    return o.id == order.id;
                }
                return o.id == order.id && o.clientOrderId == order.clientOrderId;
            };

            bool erased_one = false;
            for (auto lit = level.begin(); lit != level.end(); ++lit) {
                if (match(*lit)) {
                    level.erase(lit);
                    erased_one = true;
                    --m_orderCnt;
                    break;
                }
            }

            if (!erased_one) {
                return &it->second;
            }
            if (level.empty()) {
                m_avl.erase(px);
                if (m_low == &it->second) {
                    m_low = nullptr;
                }
                if (m_high == &it->second) {
                    m_high = nullptr;
                }

                m_levels.erase(it);
                recomputeRange();
                return nullptr;
            }
            if (!m_low || px < m_low->price) {
                m_low = &it->second;
            }
            if (!m_high || px > m_high->price) {
                m_high = &it->second;
            }

            return &it->second;
        }

        /**
         * @brief Find a price level by price (or price-like object).
         * @tparam K Type comparable to `price_type` using the AVL comparator.
         * @param price_like Price or compatible key to search.
         * @return Pointer to the corresponding level node, or nullptr if not found.
         */
        template<typename K>
        [[nodiscard]] node_ref find(const K& price_like) noexcept {
            auto n = m_avl.find(price_like);
            if (!n) {
                return nullptr;
            }

            auto it = m_levels.find(n->key);
            return (it == m_levels.end()) ? nullptr : &it->second;
        }

        /**
         * @brief Const overload of find.
         */
        template<typename K>
        [[nodiscard]] const node_ref find(const K& price_like) const noexcept {
            auto n = m_avl.find(price_like);
            if (!n) {
                return nullptr;
            }
            auto it = m_levels.find(n->key);
            return (it == m_levels.end()) ? nullptr : &it->second;
        }

        /**
         * @brief Get the top N orders from this side of the book.
         *
         * - For Buy side: highest priced orders.
         * - For Sell side: lowest priced orders.
         *
         * @param length Maximum number of top orders to return (default = 1).
         * @return Vector of pointers to the top orders (at most `length`).
         */
        [[nodiscard]] order_ptr_vec top(int length = 1) {
            order_ptr_vec out;
            if (length <= 0 || empty()) {
                return out;
            }

            out.reserve(static_cast<size_t>(length));

            std::vector<price_type> keys;
            keys.reserve(std::min<size_t>(m_levels.size(), static_cast<size_t>(length)));
            m_avl.inorder([&](auto* n) { keys.push_back(n->key); },
                          std::numeric_limits<size_t>::max());

            if (m_side == Side::Buy) {
                for (auto it = keys.rbegin();
                     it != keys.rend() && out.size() < static_cast<size_t>(length);
                     ++it) {
                    auto& node = m_levels.at(*it);
                    if (!node.level.empty()) out.push_back(&node.level.front());
                }
            } else {
                for (auto it = keys.begin();
                     it != keys.end() && out.size() < static_cast<size_t>(length);
                     ++it) {
                    auto& node = m_levels.at(*it);
                    if (!node.level.empty()) out.push_back(&node.level.front());
                }
            }
            return out;
        }

        /**
         * @brief Get pointer to the lowest-priced level.
         * @return Pointer to lowest price level, or nullptr if none.
         */
        [[nodiscard]] const node_ref low() const noexcept { return m_low; }

        /**
         * @brief Get pointer to the highest-priced level.
         * @return Pointer to highest price level, or nullptr if none.
         */
        [[nodiscard]] const node_ref high() const noexcept { return m_high; }

        /**
         * @brief Get the side of the tree (Buy or Sell).
         * @return Side of this tree.
         */
        [[nodiscard]] Side side() const noexcept { return m_side; }

        /**
         * @brief Set the side of the tree.
         * @param s Side (Buy or Sell).
         */
        void setSide(Side s) noexcept { m_side = s; }

        /**
         * @brief Print a summary of this side’s levels and order counts.
         *
         * Output format:
         * ```
         * BUY side (levels=N, orders=M)
         *   price1 -> size=K1
         *   price2 -> size=K2
         *   ...
         * ```
         *
         * @param os Output stream (defaults to std::cout).
         */
        void print(std::ostream& os = std::cout) const {
            os << (m_side == Side::Buy ? "BUY" : "SELL") << " side (levels=" << m_levels.size()
               << ", orders=" << m_orderCnt << ")\n";
            m_avl.inorder([&](const auto* n) {
                const auto px = n->key;
                auto it = m_levels.find(px);
                if (it != m_levels.end()) {
                    os << "  " << px << " -> size=" << it->second.level.size() << '\n';
                }
            });
        }

    private:
        /**
         * @brief Update low/high pointers after inserting a new level.
         * @param node Newly inserted node.
         */
        void touchRangeOnInsert(node_ref node) noexcept {
            if (!m_low || node->price < m_low->price) {
                m_low = node;
            }
            if (!m_high || node->price > m_high->price) {
                m_high = node;
            }
        }

        /**
         * @brief Recompute low/high pointers by scanning the AVL tree.
         *
         * Called when levels are removed and cached pointers may be invalid.
         */
        void recomputeRange() noexcept {
            m_low = m_high = nullptr;
            if (m_levels.empty()) {
                return;
            }
            if (auto mn = m_avl.findMin()) {
                if (auto it = m_levels.find(mn->key); it != m_levels.end()) {
                    m_low = &it->second;
                }
            }
            if (auto mx = m_avl.findMax()) {
                if (auto it = m_levels.find(mx->key); it != m_levels.end()) {
                    m_high = &it->second;
                }
            }
        }

    private:
        tree_type m_avl{};  ///< AVL tree storing price keys.
        levels_map m_levels{};  ///< Hash map of price levels.
        node_ref m_low{nullptr};  ///< Pointer to lowest price level.
        node_ref m_high{nullptr};  ///< Pointer to highest price level.
        size_t m_orderCnt{0};  ///< Total number of active orders.
        Side m_side{Side::Buy};  ///< Side of the book (Buy or Sell).
    };
}  // namespace tradestack
