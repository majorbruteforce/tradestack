#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>

#include "order.hpp"

namespace tradestack {
    struct PriceLevelNode {
        using price_type = std::uint64_t;
        using size_type = std::size_t;
        using level_type = std::list<Order>;
        using node_type = PriceLevelNode;
        using node_ptr = std::unique_ptr<node_type>;

        price_type price{};
        level_type level;
        price_type height{1};
        node_ptr left{nullptr};
        node_ptr right{nullptr};

        PriceLevelNode() noexcept = default;
        constexpr explicit PriceLevelNode(price_type p) noexcept : price(p) {}

        PriceLevelNode(const node_type&) = delete;
        node_type& operator=(const node_type&) = delete;
        PriceLevelNode(node_type&&) noexcept = default;
        node_type& operator=(node_type&&) noexcept = default;

        ~PriceLevelNode() = default;

        [[nodiscard]] constexpr bool empty() const noexcept { return level.empty(); }
        [[nodiscard]] constexpr size_type size() const noexcept { return level.size(); }
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
