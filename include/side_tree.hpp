#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "avl_tree.hpp"
#include "order.hpp"
#include "price_level_node.hpp"

namespace tradestack {
    class SideTree {
    public:
        using price_type = uint64_t;
        using order_type = Order;
        using node_type = PriceLevelNode;
        using node_ref = node_type*;
        using order_list = std::vector<order_type>;
        using order_ptr_vec = std::vector<order_type*>;

        using tree_type = AVLTree<price_type>;
        using tree_node = typename tree_type::node_type;
        using tree_node_ptr = typename tree_type::node_ref;
        using const_tree_node = typename tree_type::const_node;

        using levels_map = std::unordered_map<price_type, node_type, std::hash<price_type>,
                                              std::equal_to<price_type> >;

        using levels_iterator = typename levels_map::iterator;
        using const_levels_iterator = typename levels_map::const_iterator;

        explicit SideTree(Side side = Side::Buy) noexcept : m_side{side} {}

        SideTree(const SideTree&) = delete;
        SideTree& operator=(const SideTree&) = delete;
        SideTree(SideTree&&) noexcept = default;
        SideTree& operator=(SideTree&&) noexcept = default;

        ~SideTree() = default;

        [[nodiscard]] bool empty() const noexcept { return m_orderCnt == 0; }
        [[nodiscard]] size_t size() const noexcept { return m_orderCnt; }

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

        template<typename K>
        [[nodiscard]] node_ref find(const K& price_like) noexcept {
            auto n = m_avl.find(price_like);
            if (!n) {
                return nullptr;
            }

            auto it = m_levels.find(n->key);
            return (it == m_levels.end()) ? nullptr : &it->second;
        }

        template<typename K>
        [[nodiscard]] const node_ref find(const K& price_like) const noexcept {
            auto n = m_avl.find(price_like);
            if (!n) {
                return nullptr;
            }
            auto it = m_levels.find(n->key);
            return (it == m_levels.end()) ? nullptr : &it->second;
        }

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

        [[nodiscard]] const node_ref low() const noexcept { return m_low; }
        [[nodiscard]] const node_ref high() const noexcept { return m_high; }
        [[nodiscard]] Side side() const noexcept { return m_side; }
        void setSide(Side s) noexcept { m_side = s; }

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
        void touchRangeOnInsert(node_ref node) noexcept {
            if (!m_low || node->price < m_low->price) {
                m_low = node;
            }
            if (!m_high || node->price > m_high->price) {
                m_high = node;
            }
        }

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
        tree_type m_avl{};
        levels_map m_levels{};
        node_ref m_low{nullptr};
        node_ref m_high{nullptr};
        size_t m_orderCnt{0};
        Side m_side{Side::Buy};
    };
}  // namespace tradestack
