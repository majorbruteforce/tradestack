#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>

#include "order.hpp"

/**
 * @file price_level_node.hpp
 * @brief Defines the PriceLevelNode structure used in the order book.
 *
 * A PriceLevelNode represents a single price level in the order book.
 * Each node stores:
 * - A price
 * - A list of orders at that price
 * - Height metadata (for balancing in an AVL tree)
 * - Left/right child nodes
 */

namespace tradestack {
    /**
     * @brief Represents a single node in the price level tree.
     *
     * A `PriceLevelNode` is part of a binary search tree (typically AVL),
     * where each node corresponds to a single price level. It stores
     * a list of orders (`Order`) at that price and maintains left and right
     * child pointers to represent the book structure.
     *
     * This design allows efficient order insertion, removal, and price-level
     * traversal for matching engines.
     */
    struct PriceLevelNode {
        /// Type representing a price value.
        using price_type = uint64_t;
        /// Type representing container sizes.
        using size_type = size_t;
        /// Container type for storing orders at this price level.
        using level_type = std::list<Order>;
        /// Alias for this node type.
        using node_type = PriceLevelNode;
        /// Smart pointer to a node (unique ownership).
        using node_ptr = std::unique_ptr<node_type>;

        price_type price{};  ///< Price associated with this level.
        level_type level;  ///< List of orders at this price.
        price_type height{1};  ///< Height of this node (for AVL balancing).
        node_ptr left{nullptr};  ///< Pointer to left child node.
        node_ptr right{nullptr};  ///< Pointer to right child node.

        /**
         * @brief Default constructor.
         *
         * Initializes a node with default values (price = 0, empty level, height = 1).
         */
        PriceLevelNode() noexcept = default;

        /**
         * @brief Construct a node for a specific price.
         *
         * @param p Price value for this level.
         */
        constexpr explicit PriceLevelNode(price_type p) noexcept : price(p) {}

        /// Copy constructor is deleted (nodes are non-copyable).
        PriceLevelNode(const node_type&) = delete;
        /// Copy assignment is deleted (nodes are non-copyable).
        node_type& operator=(const node_type&) = delete;
        /// Move constructor (transfers ownership of subtrees and orders).
        PriceLevelNode(node_type&&) noexcept = default;
        /// Move assignment operator (transfers ownership of subtrees and orders).
        node_type& operator=(node_type&&) noexcept = default;
        /// Default destructor.
        ~PriceLevelNode() = default;

        /**
         * @brief Check whether this price level has no orders.
         * @return true if there are no orders at this level, false otherwise.
         */
        [[nodiscard]] constexpr bool empty() const noexcept { return level.empty(); }

        /**
         * @brief Get the number of orders at this price level.
         * @return Number of orders stored in this level.
         */
        [[nodiscard]] constexpr size_type size() const noexcept { return level.size(); }

        /**
         * @brief Create a deep copy (clone) of this node and its subtree.
         *
         * Copies:
         * - Price
         * - Orders list
         * - Height
         * - Left and right subtrees (recursively)
         *
         * @return Unique pointer to the cloned node (with full subtree).
         */
        [[nodiscard]] node_ptr clone() const {
            auto node = std::make_unique<node_type>(price);
            node->level = level;
            node->height = height;
            if (left) {
                node->left = left->clone();
            }
            if (right) {
                node->right = right->clone();
            }

            return node;
        }
    };
}  // namespace tradestack
