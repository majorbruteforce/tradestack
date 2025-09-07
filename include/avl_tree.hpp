#pragma once

#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <utility>

/**
 * @file AVLTree.hpp
 * @brief Self-balancing binary search tree implementation (AVL Tree).
 *
 * Provides insertion, deletion, search, and traversal in logarithmic time complexity.
 * This implementation supports transparent comparators, parent pointers, min/max tracking,
 * and inorder traversal with visitor callbacks.
 */

namespace tradestack {
    /**
     * @brief Concept to check whether a comparator supports heterogeneous lookup.
     *
     * This ensures the comparator `Cmp` can compare objects of type `Key` with
     * an alternative type `K`. Requires that the comparator defines an
     * `is_transparent` typedef, similar to `std::less<>`.
     *
     * @tparam Cmp Comparator type.
     * @tparam Key Primary key type.
     * @tparam K Alternative comparable key type.
     */
    template<class Cmp, class Key, class K>
    concept TransparentComparable = requires(const Cmp& cmp, const Key& a, const K& b) {
        typename Cmp::is_transparent;
        { cmp(a, b) } -> std::convertible_to<bool>;
        { cmp(b, a) } -> std::convertible_to<bool>;
    };

    /**
     * @brief Self-balancing binary search tree (AVL Tree).
     *
     * Stores elements of type `Key` in a balanced BST. Guarantees
     * logarithmic-time complexity for insertion, deletion, and search.
     *
     * @tparam Key Key type to store.
     * @tparam Compare Comparator type, defaults to `std::less<Key>`
     */
    template<typename Key, typename Compare = std::less<Key>>
    class AVLTree {
    public:
        /**
         * @brief Node structure for AVLTree.
         */
        struct Node {
            Key key{};  ///< Stored key value
            int height{1};  ///< Height of the subtree rooted at this node
            std::unique_ptr<Node> left;  ///< Pointer to left child
            std::unique_ptr<Node> right;  ///< Pointer to right child
            Node* parent{nullptr};  ///< Pointer to parent node (non-owning)

            /// Construct node from lvalue key.
            explicit Node(const Key& k) : key(k) {}
            /// Construct node from rvalue key.
            explicit Node(Key&& k) : key(std::move(k)) {}
        };

        /// Node type.
        using node_type = Node;
        /// Unique owning pointer to a node.
        using node_ptr = std::unique_ptr<node_type>;
        /// Mutable raw node pointer.
        using node_ref = node_type*;
        /// Const raw node pointer.
        using const_node = const node_type*;
        /// Function type for visiting mutable nodes.
        using visit_func = std::function<void(node_ref)>;
        /// Function type for visiting const nodes.
        using cvisit_func = std::function<void(const_node)>;
        /// Result of an insertion: (pointer to node, bool created).
        using insert_res = std::pair<node_ref, bool>;
        /// Size type for counts.
        using size_type = std::size_t;
        /// Type representing node height.
        using height_type = int;

        /// Maximum limit for visit functions.
        static constexpr size_type MAX_VISIT = std::numeric_limits<size_type>::max();

        /// Default constructor.
        AVLTree() = default;
        /// Deleted copy constructor.
        AVLTree(const AVLTree&) = delete;
        /// Deleted copy assignment.
        AVLTree& operator=(const AVLTree&) = delete;

        /**
         * @brief Move constructor.
         */
        AVLTree(AVLTree&& other) noexcept :
            m_root(std::move(other.m_root)), m_cmp(std::move(other.m_cmp)), m_min(other.m_min),
            m_max(other.m_max) {
            other.m_min = other.m_max = nullptr;
        }

        /**
         * @brief Move assignment operator.
         */
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

        /// Default destructor.
        ~AVLTree() = default;

        /// @return true if tree is empty.
        [[nodiscard]] bool empty() const noexcept { return !m_root; }

        /// @return true if tree has a root node.
        [[nodiscard]] bool has_root() const noexcept { return static_cast<bool>(m_root); }

        /// Clear all nodes from the tree.
        void reset() {
            m_root.reset();
            m_min = m_max = nullptr;
        }

        /// Insert a key by copy.
        [[nodiscard]] insert_res insert(const Key& k) { return insert_impl(m_root, k); }
        /// Insert a key by move.
        [[nodiscard]] insert_res insert(Key&& k) { return insert_impl(m_root, std::move(k)); }

        /// Erase a key by value. Returns true if erased.
        bool erase(const Key& k) { return erase_impl(m_root, k); }

        /// Find a node by key. Mutable.
        [[nodiscard]] node_ref find(const Key& k) noexcept { return find_impl(m_root.get(), k); }
        /// Find a node by key. Const.
        [[nodiscard]] const_node find(const Key& k) const noexcept {
            return find_impl(m_root.get(), k);
        }

        /**
         * @brief Insert with transparent comparator.
         *
         * Allows insertion with a key type `K` different from `Key` if
         * the comparator supports it.
         */
        template<class K>
            requires TransparentComparable<Compare, Key, K> &&
                     (!std::same_as<std::remove_cvref_t<K>, Key>)
        [[nodiscard]] insert_res insert(K&& k) {
            return insert_impl(m_root, std::forward<K>(k));
        }

        /**
         * @brief Erase with transparent comparator.
         */
        template<class K>
            requires TransparentComparable<Compare, Key, K> &&
                     (!std::same_as<std::remove_cvref_t<K>, Key>)
        bool erase(const K& k) {
            return erase_impl(m_root, k);
        }

        /**
         * @brief Erase a node directly by pointer.
         *
         * @param n Node pointer to erase.
         * @return true if erased.
         */
        bool erase(node_ref n) { return n ? erase(n->key) : false; }

        /**
         * @brief Find with transparent comparator.
         */
        template<class K>
            requires TransparentComparable<Compare, Key, K> &&
                     (!std::same_as<std::remove_cvref_t<K>, Key>)
        [[nodiscard]] node_ref find(const K& k) noexcept {
            return find_impl(m_root.get(), k);
        }

        /**
         * @brief Const find with transparent comparator.
         */
        template<class K>
            requires TransparentComparable<Compare, Key, K> &&
                     (!std::same_as<std::remove_cvref_t<K>, Key>)
        [[nodiscard]] const_node find(const K& k) const noexcept {
            return find_impl(m_root.get(), k);
        }

        /// @return pointer to smallest node, or nullptr if empty.
        [[nodiscard]] const_node findMin() const noexcept { return m_min; }

        /// @return pointer to largest node, or nullptr if empty.
        [[nodiscard]] const_node findMax() const noexcept { return m_max; }

        /**
         * @brief Inorder traversal (mutable visitor).
         *
         * @param visit Visitor function taking `node_ref`.
         * @param limit Maximum number of nodes to visit (default: all).
         */
        template<typename F>
        void inorder(F&& visit, size_type limit = MAX_VISIT) {
            size_type count = 0;
            inorder_impl(m_root.get(), std::forward<F>(visit), limit, count);
        }

        /**
         * @brief Inorder traversal (const visitor).
         *
         * @param visit Visitor function taking `const_node`.
         * @param limit Maximum number of nodes to visit (default: all).
         */
        template<typename F>
        void inorder(F&& visit, size_type limit = MAX_VISIT) const {
            size_type count = 0;
            inorder_impl(m_root.get(), std::forward<F>(visit), limit, count);
        }

    private:
        node_ptr m_root;  ///< Root node
        [[no_unique_address]] Compare m_cmp;  ///< Comparator
        node_ref m_min{nullptr};  ///< Cached pointer to minimum node
        node_ref m_max{nullptr};  ///< Cached pointer to maximum node

        /// @return height of node (0 if null).
        static height_type height(const_node n) noexcept { return n ? n->height : 0; }

        /// Update node height based on children.
        static void update_height(node_ref n) noexcept {
            n->height = 1 + std::max(height(n->left.get()), height(n->right.get()));
        }

        /// @return balance factor (left height - right height).
        static height_type balance_factor(const_node n) noexcept {
            return height(n->left.get()) - height(n->right.get());
        }

        /**
         * @brief Perform a left rotation around the given subtree root.
         *
         * Transform:
         * ```
         *     p                  y
         *      \                / \
         *       y     --->     p   γ
         *      / \              \
         *     β   γ              β
         * ```
         *
         * @param p Reference to the unique_ptr pointing to the subtree root.
         *
         * Updates parent pointers and heights accordingly.
         */
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

        /**
         * @brief Perform a right rotation around the given subtree root.
         *
         * Transform:
         * ```
         *        p              y
         *       /              / \
         *      y     --->     α   p
         *     / \                /
         *    α   β              β
         * ```
         *
         * @param p Reference to the unique_ptr pointing to the subtree root.
         *
         * Updates parent pointers and heights accordingly.
         */
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

        /**
         * @brief Rebalance a subtree after insertion or deletion.
         *
         * Updates node height, computes balance factor, and applies
         * appropriate rotations:
         * - Left-Left case → rotate_right
         * - Left-Right case → rotate_left then rotate_right
         * - Right-Right case → rotate_left
         * - Right-Left case → rotate_right then rotate_left
         *
         * @param p Reference to the unique_ptr pointing to the subtree root.
         */
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

        /**
         * @brief Recursive insertion helper.
         *
         * @tparam K Key type (may be `Key` or compatible type via transparent comparator).
         * @param p Subtree root reference.
         * @param k Key to insert.
         * @param parent Parent node pointer.
         *
         * @return Pair (node_ref inserted/found, bool created).
         */
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

        /**
         * @brief Recursive erase helper.
         *
         * @tparam K Key type.
         * @param p Subtree root reference.
         * @param k Key to erase.
         * @return true if key was found and erased.
         */
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

        /**
         * @brief Erase a node by pointer.
         *
         * Handles three cases:
         * - Node with no children
         * - Node with one child
         * - Node with two children (replace with inorder successor)
         *
         * Updates min/max tracking and rebalances.
         *
         * @param p Reference to node unique_ptr.
         * @return true if erased.
         */
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

        /**
         * @brief Find the in-order successor of a node.
         *
         * Successor = smallest node greater than `n`.
         *
         * @param n Input node.
         * @return Pointer to successor node, or nullptr if none.
         */
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

        /**
         * @brief Find the in-order predecessor of a node.
         *
         * Predecessor = largest node smaller than `n`.
         *
         * @param n Input node.
         * @return Pointer to predecessor node, or nullptr if none.
         */
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

        /**
         * @brief Recursive search helper (mutable).
         *
         * @tparam K Key type.
         * @param n Subtree root.
         * @param k Key to search for.
         * @return Node pointer if found, nullptr otherwise.
         */
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

        /**
         * @brief Recursive search helper (const).
         *
         * @tparam K Key type.
         * @param n Subtree root.
         * @param k Key to search for.
         * @return Const node pointer if found, nullptr otherwise.
         */
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

        /**
         * @brief In-order traversal helper (mutable).
         *
         * Visits nodes in sorted order.
         *
         * @tparam F Visitor function type.
         * @param n Subtree root.
         * @param visit Visitor callable taking node_ref.
         * @param limit Maximum number of nodes to visit.
         * @param count Reference to visited count.
         */
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

        /**
         * @brief In-order traversal helper (const).
         *
         * Visits nodes in sorted order.
         *
         * @tparam F Visitor function type.
         * @param n Subtree root.
         * @param visit Visitor callable taking const_node.
         * @param limit Maximum number of nodes to visit.
         * @param count Reference to visited count.
         */
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
