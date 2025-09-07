#pragma once

#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <utility>

namespace tradestack {
    template<class Cmp, class Key, class K>
    concept TransparentComparable = requires(const Cmp& cmp, const Key& a, const K& b) {
        typename Cmp::is_transparent;
        { cmp(a, b) } -> std::convertible_to<bool>;
        { cmp(b, a) } -> std::convertible_to<bool>;
    };

    template<typename Key, typename Compare = std::less<>>
    class AVLTree {
    public:
        struct Node {
            Key key{};
            int height{1};
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;
            Node* parent{nullptr};

            explicit Node(const Key& k) : key(k) {}
            explicit Node(Key&& k) : key(std::move(k)) {}
        };

        using node_type = Node;
        using node_ptr = std::unique_ptr<node_type>;
        using node_ref = node_type*;
        using const_node = const node_type*;
        using visit_func = std::function<void(node_ref)>;
        using cvisit_func = std::function<void(const_node)>;
        using insert_res = std::pair<node_ref, bool>;
        using size_type = std::size_t;
        using height_type = int;

        static constexpr size_type MAX_VISIT = std::numeric_limits<size_type>::max();

        AVLTree() = default;
        AVLTree(const AVLTree&) = delete;
        AVLTree& operator=(const AVLTree&) = delete;

        AVLTree(AVLTree&& other) noexcept :
            m_root(std::move(other.m_root)), m_cmp(std::move(other.m_cmp)), m_min(other.m_min),
            m_max(other.m_max) {
            other.m_min = other.m_max = nullptr;
        }

        AVLTree& operator=(AVLTree&& other) noexcept {
            if (this != &other) {
                m_root = std::move(other.m_root);
                m_cmp = std::move(other.m_cmp);
                m_min = other.m_min;
                m_max = other.m_max;
                other.m_min = other.m_max = nullptr;
            }
            return *this;
        }

        ~AVLTree() = default;

        [[nodiscard]] bool empty() const noexcept { return !m_root; }
        [[nodiscard]] bool has_root() const noexcept { return static_cast<bool>(m_root); }

        void reset() {
            m_root.reset();
            m_min = m_max = nullptr;
        }

        [[nodiscard]] insert_res insert(const Key& k) { return insert_impl(m_root, k); }
        [[nodiscard]] insert_res insert(Key&& k) { return insert_impl(m_root, std::move(k)); }

        bool erase(const Key& k) { return erase_impl(m_root, k); }

        [[nodiscard]] node_ref find(const Key& k) noexcept { return find_impl(m_root.get(), k); }
        [[nodiscard]] const_node find(const Key& k) const noexcept {
            return find_impl(m_root.get(), k);
        }

        template<class K>
            requires TransparentComparable<Compare, Key, K> &&
                     (!std::same_as<std::remove_cvref_t<K>, Key>)
        [[nodiscard]] insert_res insert(K&& k) {
            return insert_impl(m_root, std::forward<K>(k));
        }

        template<class K>
            requires TransparentComparable<Compare, Key, K> &&
                     (!std::same_as<std::remove_cvref_t<K>, Key>)
        bool erase(const K& k) {
            return erase_impl(m_root, k);
        }

        bool erase(node_ref n) { return n ? erase(n->key) : false; }

        template<class K>
            requires TransparentComparable<Compare, Key, K> &&
                     (!std::same_as<std::remove_cvref_t<K>, Key>)
        [[nodiscard]] node_ref find(const K& k) noexcept {
            return find_impl(m_root.get(), k);
        }

        template<class K>
            requires TransparentComparable<Compare, Key, K> &&
                     (!std::same_as<std::remove_cvref_t<K>, Key>)
        [[nodiscard]] const_node find(const K& k) const noexcept {
            return find_impl(m_root.get(), k);
        }

        [[nodiscard]] const_node findMin() const noexcept { return m_min; }
        [[nodiscard]] const_node findMax() const noexcept { return m_max; }

        template<typename F>
        void inorder(F&& visit, size_type limit = MAX_VISIT) {
            size_type count = 0;
            inorder_impl(m_root.get(), std::forward<F>(visit), limit, count);
        }

        template<typename F>
        void inorder(F&& visit, size_type limit = MAX_VISIT) const {
            size_type count = 0;
            inorder_impl(m_root.get(), std::forward<F>(visit), limit, count);
        }

    private:
        node_ptr m_root;
        [[no_unique_address]] Compare m_cmp;
        node_ref m_min{nullptr};
        node_ref m_max{nullptr};

        static height_type height(const_node n) noexcept { return n ? n->height : 0; }

        static void update_height(node_ref n) noexcept {
            n->height = 1 + std::max(height(n->left.get()), height(n->right.get()));
        }

        static height_type balance_factor(const_node n) noexcept {
            return height(n->left.get()) - height(n->right.get());
        }

        static void rotate_left(node_ptr& p) {
            node_ref old_parent = p ? p->parent : nullptr;
            node_ptr y = std::move(p->right);
            node_ptr beta = std::move(y->left);

            y->left = std::move(p);
            y->left->parent = y.get();
            y->left->right = std::move(beta);
            if (y->left->right) {
                y->left->right->parent = y->left.get();
            }

            p = std::move(y);
            p->parent = old_parent;

            update_height(p->left.get());
            update_height(p.get());
        }

        static void rotate_right(node_ptr& p) {
            node_ref old_parent = p ? p->parent : nullptr;
            node_ptr y = std::move(p->left);
            node_ptr beta = std::move(y->right);

            y->right = std::move(p);
            y->right->parent = y.get();
            y->right->left = std::move(beta);
            if (y->right->left) {
                y->right->left->parent = y->right.get();
            }

            p = std::move(y);
            p->parent = old_parent;

            update_height(p->right.get());
            update_height(p.get());
        }

        static void rebalance(node_ptr& p) {
            if (!p) {
                return;
            }

            update_height(p.get());
            height_type bf = balance_factor(p.get());

            if (bf > 1) {
                if (balance_factor(p->left.get()) < 0) rotate_left(p->left);
                rotate_right(p);
            } else if (bf < -1) {
                if (balance_factor(p->right.get()) > 0) rotate_right(p->right);
                rotate_left(p);
            }
        }

        template<typename K>
        insert_res insert_impl(node_ptr& p, K&& k, Node* parent = nullptr) {
            if (!p) {
                p = std::make_unique<node_type>(std::forward<K>(k));
                p->parent = parent;
                if (!m_min || m_cmp(k, m_min->key)) {
                    m_min = p.get();
                }
                if (!m_max || m_cmp(m_max->key, k)) {
                    m_max = p.get();
                }

                return {p.get(), true};
            }
            if (m_cmp(k, p->key)) {
                auto [n, created] = insert_impl(p->left, std::forward<K>(k), p.get());
                rebalance(p);
                return {n, created};
            } else if (m_cmp(p->key, k)) {
                auto [n, created] = insert_impl(p->right, std::forward<K>(k), p.get());
                rebalance(p);
                return {n, created};
            } else {
                return {p.get(), false};
            }
        }

        template<typename K>
        bool erase_impl(node_ptr& p, const K& k) {
            if (!p) {
                return false;
            }
            if (m_cmp(k, p->key)) {
                if (erase_impl(p->left, k)) {
                    rebalance(p);
                    return true;
                }
                return false;
            } else if (m_cmp(p->key, k)) {
                if (erase_impl(p->right, k)) {
                    rebalance(p);
                    return true;
                }
                return false;
            } else {
                erase_node(p);
                return true;
            }
        }

        bool erase_node(node_ptr& p) {
            if (!p) {
                return false;
            }
            if (p.get() == m_min) {
                m_min = successor(p.get());
            }
            if (p.get() == m_max) {
                m_max = predecessor(p.get());
            }
            if (!p->left) {
                node_ptr r = std::move(p->right);
                if (r) {
                    r->parent = p->parent;
                }
                p = std::move(r);
            } else if (!p->right) {
                node_ptr l = std::move(p->left);
                if (l) {
                    l->parent = p->parent;
                }
                p = std::move(l);
            } else {
                node_ref s = p->right.get();
                while (s->left) s = s->left.get();
                p->key = s->key;
                erase_impl(p->right, s->key);
            }
            if (p) {
                rebalance(p);
            }

            return true;
        }

        static node_ref successor(node_ref n) {
            if (!n) {
                return nullptr;
            }
            if (n->right) {
                n = n->right.get();
                while (n->left) {
                    n = n->left.get();
                }
                return n;
            }

            Node* up = n->parent;
            while (up && n == up->right.get()) {
                n = up;
                up = up->parent;
            }
            return up;
        }

        static node_ref predecessor(node_ref n) {
            if (!n) {
                return nullptr;
            }
            if (n->left) {
                n = n->left.get();
                while (n->right) {
                    n = n->right.get();
                }
                return n;
            }

            Node* up = n->parent;
            while (up && n == up->left.get()) {
                n = up;
                up = up->parent;
            }
            return up;
        }

        template<typename K>
        node_ref find_impl(node_ref n, const K& k) noexcept {
            while (n) {
                if (!m_cmp(n->key, k) && !m_cmp(k, n->key)) {
                    return n;
                }

                n = m_cmp(k, n->key) ? n->left.get() : n->right.get();
            }
            return nullptr;
        }

        template<typename K>
        const_node find_impl(const_node n, const K& k) const noexcept {
            while (n) {
                if (!m_cmp(n->key, k) && !m_cmp(k, n->key)) {
                    return n;
                }

                n = m_cmp(k, n->key) ? n->left.get() : n->right.get();
            }
            return nullptr;
        }

        template<class F>
        static void inorder_impl(node_ref n, F&& visit, size_type limit, size_type& count) {
            if (!n || count >= limit) {
                return;
            }
            inorder_impl(n->left.get(), std::forward<F>(visit), limit, count);
            if (count < limit) {
                std::forward<F>(visit)(n);
                ++count;
            }

            inorder_impl(n->right.get(), std::forward<F>(visit), limit, count);
        }

        template<class F>
        static void inorder_impl(const_node n, F&& visit, size_type limit, size_type& count) {
            if (!n || count >= limit) {
                return;
            }
            inorder_impl(n->left.get(), std::forward<F>(visit), limit, count);
            if (count < limit) {
                std::forward<F>(visit)(n);
                ++count;
            }

            inorder_impl(n->right.get(), std::forward<F>(visit), limit, count);
        }
    };
}  // namespace tradestack
