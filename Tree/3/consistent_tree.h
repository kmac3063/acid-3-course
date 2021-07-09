#pragma once

#include <thread>
#include <mutex>
#include <shared_mutex>

struct receiver {
    int value = 0;
};

template<typename T>
class consistent_tree {
public:
    using height_t = uint8_t;
    using value_t = T;
    using ref_count_t = uint32_t;

    class node;

    class value_node;

    class iterator;

    class node {
    private:
        value_t value;
        height_t height = 1;

        node *left = nullptr;
        node *right = nullptr;
        node *parent = nullptr;

        bool is_deleted_ = false;
        ref_count_t ref_count = 0;

    public:
        consistent_tree *tree;

        node(consistent_tree *tree_, node *parent_, value_t value_) :
                tree(tree_), value(value_), parent(parent_) {}

        value_t get_value() {
            return value;
        }

        void set_value(const value_t &value_) {
            value = value_;
        }


        height_t get_height() {
            return height;
        }

        void set_height(const height_t &height_) {
            height = height_;
        }


        node *get_left() {
            return left;
        }

        void set_left(node *node_) {
            left = node_;
            if (node_ != nullptr) {
                node_->set_parent(this);
            }
        }


        node *get_right() {
            return right;
        }

        void set_right(node *node_) {
            right = node_;
            if (node_ != nullptr) {
                node_->set_parent(this);
            }
        }


        node *get_parent() {
            return parent;
        }

        void set_parent(node *node_) {
            parent = node_;
        }


        void add_ref_count(int n) {
            if (this == tree->HEAD_NODE) {
                return;
            }

            ref_count += n;

            if (need_free()) {
                tree->finally_erase(value);
            }
        }

        void set_deleted(bool delete_flag) {
            if (is_deleted_ && !delete_flag) {
                tree->size_++;
            } else if (!is_deleted_ && delete_flag) {
                tree->size_--;
            }

            is_deleted_ = delete_flag;

            if (need_free()) {
                tree->finally_erase(value);
            }
        }

        bool is_deleted() {
            return is_deleted_;
        }


        void free() {
            unlink();
            tree->n_deleted_node++;
//            std::cout << "It's all, we deleted :( value = " << value << std::endl;
            delete this;
        }

        void unlink() {
            if (this == get_parent()->get_left()) {
                get_parent()->set_left(nullptr);
            } else {
                get_parent()->set_right(nullptr);
            }

            if (get_left() != nullptr) {
                get_left()->set_parent(nullptr);
            }

            if (get_right() != nullptr) {
                get_right()->set_parent(nullptr);
            }

            left = right = parent = nullptr;
        }

        bool need_free() {
            return is_deleted_ && ref_count == 0;
        }
    };

    class value_node {
    private:
        node *current_node;
    public:
        explicit value_node(node *node_) : current_node(node_) {}

        value_t get() {
            return current_node->get_value();
        }

        void set(const value_t &value_) {
            current_node->set_value(value_);
        }
    };


    node *HEAD_NODE;
    uint32_t n_deleted_node = 0;

    receiver *deleted_node_receiver = nullptr;

    size_t size_ = 0;
    std::shared_mutex mutex_;


    consistent_tree() {
        HEAD_NODE = new node(this, nullptr, value_t());
        HEAD_NODE->set_parent(HEAD_NODE);
    }

    explicit consistent_tree(receiver *receiver_) : consistent_tree() {
        deleted_node_receiver = receiver_;
    }

    consistent_tree(const consistent_tree &tree_) {
        HEAD_NODE = new node(this, nullptr, value_t());
        HEAD_NODE->set_parent(HEAD_NODE);

        add_all(tree_.HEAD_NODE->get_right());
    }

    consistent_tree &operator=(const consistent_tree &tree_) {
        if (this != &tree_) {
            HEAD_NODE = new node(this, nullptr, value_t());
            HEAD_NODE->set_parent(HEAD_NODE);
            n_deleted_node = 0;

            add_all(tree_.HEAD_NODE->get_right());
        }

        return *this;
    }

    ~consistent_tree() {
        cascade_delete_node(HEAD_NODE);
    }


    void add_all(node *node_) {
        if (node_ == nullptr) {
            return;
        }

        add_all(node_->get_left());
        add_all(node_->get_right());

        insert(node_->get_value());
    }

    void cascade_delete_node(node *node_) {
        if (node_ == nullptr) {
            return;
        }

        cascade_delete_node(node_->get_left());
        cascade_delete_node(node_->get_right());

        n_deleted_node++;
        if (node_ == HEAD_NODE && deleted_node_receiver != nullptr) {
            deleted_node_receiver->value = n_deleted_node;
        }
        delete node_;
    }


    void insert(const value_t &value_) {
        std::unique_lock lock(mutex_);
        HEAD_NODE->set_right(insert(HEAD_NODE->get_right(), HEAD_NODE, value_));
    }

    void erase(const value_t &value_) {
        std::unique_lock lock(mutex_);
        try_remove(HEAD_NODE->get_right(), value_);
    }

    void erase(const iterator &it) {
        std::unique_lock lock(mutex_);
        try_remove(HEAD_NODE->get_right(), (*it).get());
    }

    iterator find(const value_t &value_) {
        std::shared_lock lock(mutex_);
        return iterator(find(HEAD_NODE->get_right(), value_));
    }

    bool empty() {
        std::shared_lock lock(mutex_);
        return size_ == 0;
    }

    value_t front() {
        std::shared_lock lock(mutex_);
        node *res = find_min(HEAD_NODE->get_right());
        if (res->is_deleted()) {
            res = find_next(res);
        }
        return res->get_value();
    }

    value_t back() {
        std::shared_lock lock(mutex_);
        node *res = find_max(HEAD_NODE->get_right());
        if (res->is_deleted()) {
            res = find_prev(res);
        }
        return res->get_value();
    }

    size_t size() {
        std::shared_lock lock(mutex_);
        return size_;
    }

    void clear() {
        std::unique_lock lock(mutex_);
        HEAD_NODE->set_right(nullptr);
    }


    iterator begin() {
        std::shared_lock lock(mutex_);
        node *node_ = HEAD_NODE->get_right();

        if (node_ == nullptr) {
            return iterator(HEAD_NODE);
        }

        node *min = find_min(node_);
        return iterator(min->is_deleted() ? find_next(min) : min);
    }

    iterator end() {
        std::shared_lock lock(mutex_);
        return iterator(HEAD_NODE);
    }


    std::vector<value_t> to_vector() {
        std::shared_lock lock(mutex_);
        std::vector<value_t> v;
        to_vector_(v, HEAD_NODE->get_right());
        return v;
    }


    void to_vector_(std::vector<value_t> &v, node *node_) {
        if (node_ == nullptr) {
            return;
        }
        to_vector_(v, node_->get_left());

        if (!node_->is_deleted()) {
            v.push_back(node_->get_value());
        }

        to_vector_(v, node_->get_right());
    }

    height_t get_height(node *node_) {
        return node_ == nullptr ? 0 : node_->get_height();
    }

    int bfactor(node *node_) {
        return get_height(node_->get_right()) - get_height(node_->get_left());
    }

    void fix_height(node *node_) {
        height_t lh = get_height(node_->get_left());
        height_t rh = get_height(node_->get_right());

        height_t max_h = lh > rh ? lh : rh;
        node_->set_height(max_h + 1);
    }

    static void acquire(node **dest, node *from) {
        if (dest != nullptr && *dest != nullptr) {
            (*dest)->add_ref_count(-1);
        }
        *dest = from;
        if (from != nullptr) {
            (*dest)->add_ref_count(1);
        }
    }

    node *rotate_right(node *p) {
        node *q = p->get_left();
        p->set_left(q->get_right());
        q->set_right(p);

        fix_height(p);
        fix_height(q);

        return q;
    }

    node *rotate_left(node *q) {
        node *p = q->get_right();
        q->set_right(p->get_left());
        p->set_left(q);

        fix_height(q);
        fix_height(p);

        return p;
    }

    node *balance(node *p) {
        fix_height(p);

        if (bfactor(p) == 2) {
            if (bfactor(p->get_right()) < 0) {
                node *res = rotate_right(p->get_right());
                p->set_right(res);
            }
            return rotate_left(p);
        }
        if (bfactor(p) == -2) {
            if (bfactor(p->get_left()) > 0) {
                node *res = rotate_left(p->get_left());
                p->set_left(res);
            }
            return rotate_right(p);
        }
        return p;
    }


    node *insert(node *node_, node *parent, const value_t &value_) {
        if (node_ == nullptr) {
            size_++;
            return new node(this, parent, value_);
        }
        if (value_ < node_->get_value()) {
            node_->set_left(insert(node_->get_left(), node_, value_));
        } else if (value_ > node_->get_value()) {
            node_->set_right(insert(node_->get_right(), node_, value_));
        } else {
            node_->set_deleted(false);
            return node_;
        }

        return balance(node_);
    }

    node *remove_min(node *p) {
        if (p->get_left() == nullptr)
            return p->get_right();

        p->set_left(remove_min(p->get_left()));
        return balance(p);
    }

    void try_remove(node *node_, const value_t &value_) {
        if (node_ == nullptr) {
            return;
        }

        if (node_->get_value() == value_) {
            node_->set_deleted(true);
            return;
        }

        if (value_ < node_->get_value()) {
            try_remove(node_->get_left(), value_);
        } else {
            try_remove(node_->get_right(), value_);
        }
    }

    void finally_erase(const value_t &value_) {
        node *res = finally_erase_(HEAD_NODE->get_right(), value_);
        HEAD_NODE->set_right(res);
    }

    node *finally_erase_(node *node_, const value_t &value_) {
        if (node_ == nullptr) {
            return nullptr;
        }

        if (value_ < node_->get_value()) {
            node_->set_left(finally_erase_(node_->get_left(), value_));
        } else if (value_ > node_->get_value()) {
            node_->set_right(finally_erase_(node_->get_right(), value_));
        } else {
            node *left = node_->get_left();
            node *right = node_->get_right();

            node_->free();

            if (right == nullptr) {
                return left;
            }

            node *min = find_min(right);
            min->set_right(remove_min(right));
            min->set_left(left);

            return balance(min);
        }
        return balance(node_);
    }

    node *find(node *node_, const value_t &value_) {
        if (node_ == nullptr) {
            return HEAD_NODE;
        }
        if (node_->get_value() < value_) {
            return find(node_->get_right(), value_);
        } else if (node_->get_value() > value_) {
            return find(node_->get_left(), value_);
        }

        return node_->is_deleted() ? HEAD_NODE : node_;
    }

    static node *find_min(node *node_) {
        return node_->get_left() == nullptr ? node_ : find_min(node_->get_left());
    }

    static node *find_max(node *node_) {
        return node_->get_right() == nullptr ? node_ : find_max(node_->get_right());
    }

    static node *find_next(node *node_) {
        if (node_ == node_->get_parent()) {
            return node_;
        }
        value_t value = node_->get_value();

        node *right = node_->get_right();
        if (right != nullptr) {
            auto res = find_min(right);
            return res->is_deleted() ? find_next(res) : res;
        }

        auto parent = node_->get_parent();
        while (parent->get_value() < value && parent != parent->get_parent()) {
            parent = parent->get_parent();
        }

        return parent->is_deleted() ? find_next(parent) : parent;
    }

    static node *find_prev(node *node_) {
        if (node_ == node_->get_parent()) {
            if (node_->tree->size_ == 0) {
                return node_;
            }
            node *res = find_max(node_->tree->HEAD_NODE->get_right());
            return res->is_deleted() ? find_prev(res) : res;
        }
        value_t value = node_->get_value();

        node *left = node_->get_left();
        if (left != nullptr) {
            auto res = find_max(left);
            return res->is_deleted() ? find_prev(res) : res;
        }

        auto parent = node_->get_parent();
        while (parent->get_value() > value && parent != parent->get_parent()) {
            parent = parent->get_parent();
        }

        return parent->is_deleted() ? find_prev(parent) : parent;
    }

    class iterator {
    private:
        node *current_node = nullptr;
    public:
        iterator() = default;

        explicit iterator(node *node_) {
            acquire(&current_node, node_);
        }

        iterator(const iterator &it) {
            acquire(&current_node, it.current_node);
        }

        iterator &operator=(const iterator &it) {
            if (this != &it) {
                acquire(&current_node, it.current_node);
            }

            return *this;
        }

        ~iterator() {
            if (current_node != nullptr) {
                auto v = current_node->get_value();
                current_node->add_ref_count(-1);
            }
        }

        value_node operator*() const {
            return value_node(current_node);
        }

        iterator operator++() {
            node *next = find_next(current_node);
            acquire(&current_node, next);
            return iterator(next);
        }

        iterator operator--() {
            node *prev = find_prev(current_node);
            acquire(&current_node, prev);
            return iterator(prev);
        }

        bool operator==(const iterator &rhs) {
            return &(*this->current_node) == &(*rhs.current_node);
        }

        bool operator!=(const iterator &rhs) {
            return &(*this->current_node) != &(*rhs.current_node);
        }
    };
};