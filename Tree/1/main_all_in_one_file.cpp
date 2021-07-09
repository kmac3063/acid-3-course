#include <iostream>
#include <random>
#include <vector>
#include <set>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>

/** fail_printer.h */
class fail_printer {
private:
    using csr = const std::string &;
public:
    static void print(csr file_name, csr test_name, csr reason) {
        printf("Fail. File: %10s. Test: %s. ", file_name.c_str(), test_name.c_str());

        if (!reason.empty()) {
            printf("Reason: %s.", reason.c_str());
        }
        printf("\n");
    }
};
/** fail_printer.h */

/** consistent_tree.h */
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
        value_node(node *node_) : current_node(node_) {}

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

//    std::mutex mutex_;

    consistent_tree() {
        HEAD_NODE = new node(this, nullptr, value_t());
        HEAD_NODE->set_parent(HEAD_NODE);
    }

    consistent_tree(receiver *receiver_) : consistent_tree() {
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
        HEAD_NODE->set_right(insert(HEAD_NODE->get_right(), HEAD_NODE, value_));
    }

    void erase(const value_t &value_) {
        try_remove(HEAD_NODE->get_right(), value_);
    }

    void erase(const iterator& it) {
        try_remove(HEAD_NODE->get_right(), (*it).get());
    }

    iterator find(const value_t &value_) {
        return iterator(find(HEAD_NODE->get_right(), value_));
    }

    bool empty() {
        return size_ == 0;
    }

    value_t front() {
        node* res = find_min(HEAD_NODE->get_right());
        if (res->is_deleted()) {
            res = find_next(res);
        }
        return res->get_value();
    }

    value_t back() {
        node* res = find_max(HEAD_NODE->get_right());
        if (res->is_deleted()) {
            res = find_prev(res);
        }
        return res->get_value();
    }

    size_t size() {
        return size_;
    }

    void clear() {
        HEAD_NODE->set_right(nullptr);
    }


    iterator begin() {
        node *node_ = HEAD_NODE->get_right();

        if (node_ == nullptr) {
            return iterator(HEAD_NODE);
        }

        node *min = find_min(node_);
        return iterator(min->is_deleted() ? find_next(min) : min);
    }

    iterator end() {
        return iterator(HEAD_NODE);
    }


    std::vector<value_t> to_vector() {
        std::vector<value_t> v;
        for (auto it = begin(); it != end(); ++it) {
            v.push_back((*it).get());
        }

        return v;
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

    static node *find_prev(node* node_) {
        if (node_ == node_->get_parent()) {
            if (node_->tree->size_ == 0) {
                return node_;
            }
            node* res = find_max(node_->tree->HEAD_NODE->get_right());
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

        iterator(node *node_) {
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
/** consistent_tree.h */

/** utils.h */
template<typename T>
std::set<T> v_to_set(const std::vector<T> &v) {
    return std::set(v.begin(), v.end());
}

template<typename T>
std::vector<T> set_to_vector(const std::set<T> &s) {
    return std::vector(s.begin(), s.end());
}

template<typename T>
typename consistent_tree<T>::node *get_node(consistent_tree<T> *tree,
                                            typename consistent_tree<T>::node *parent, const T &value) {
    return new typename consistent_tree<T>::node(tree, parent, value);
}

template<typename T>
consistent_tree<T> get_fake_tree(const std::vector<T> &v) {
    consistent_tree<T> tree;
    if (v.empty()) {
        return tree;
    }

    using node_type = typename consistent_tree<T>::node;

    std::vector<node_type *> node_list(v.size());
    for (int i = 0; i < v.size(); ++i) {
        node_list[i] = new node_type(&tree, nullptr, v[i]);
    }

    for (int i = (int) v.size() - 1; i >= 0; --i) {
        if (2 * i + 1 < v.size()) {
            node_list[i]->set_left(node_list[2 * i + 1]);
        }
        if (2 * i + 2 < v.size()) {
            node_list[i]->set_right(node_list[2 * i + 2]);
        }
    }

    tree.HEAD_NODE->set_right(node_list[0]);
    return tree;
}

template<typename T>
consistent_tree<T> get_fake_tree(const std::initializer_list<T> &il) {
    return get_fake_tree(std::vector(il));
}

std::vector<int> get_fake_vector_tree(int height) {
    std::vector<int> v((1 << height) - 1);
    v[0] = (1 << (height - 1));
    for (int i = 0; i < v.size() / 2; ++i) {
        int parent_value = i == 0 ? (1 << height) : v[(i - 1) / 2];
        int delta = std::abs(parent_value - v[i]) / 2;
        v[2 * i + 1] = v[i] - delta;
        v[2 * i + 2] = v[i] + delta;
    }
    return v;
}

std::vector<int> get_random_vector(int size, int start_value = 0) {
    std::vector<int> v(size);
    for (int i = 0; i < v.size(); ++i) {
        v[i] = start_value + i;
    }

//    unsigned num = 0;
    unsigned num = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(v.begin(), v.end(), std::default_random_engine(num));
    return v;
}
/** utils.h */

/** tree_test.h */
class tree_test {
private:
    std::string test_case;
    size_t test_counter = 0;
    size_t fail_counter = 0;

    void REQUIRE(bool result, const std::string &reason = "") {
        test_counter++;
        if (!result) {
            fail_counter++;
            fail_printer::print("tree_test.h", test_case, reason);
        } else {
/*            std::string s = "[test_case: " + test_case;
            if (!reason.empty()) {
                s += ", reason: " + reason;
            }
            s += "] PASSSED";
            std::cout << s << std::endl;*/
        }
    }


    void to_vector() {
        test_case = "to_vector";
        std::vector<std::vector<int>> tests = {
                {},
                {4, 2, 6, 1, 3, 5, 7},
                get_fake_vector_tree(2),
                get_fake_vector_tree(10),
                get_fake_vector_tree(10),
                get_fake_vector_tree(13),
//                get_fake_vector_tree(16),
        };
        int n_test = 1;
        for (auto &test: tests) {
            auto tree = get_fake_tree(test);
            sort(test.begin(), test.end());
            REQUIRE(tree.to_vector() == test, "case " + std::to_string(n_test));
            n_test++;
        }
    }


    void insert() {
        test_case = "insert";
        std::vector<std::vector<int>> tests = {
                {},
                {-100, 100},
                {1, 2, 3, 4, 5, 6},
                {3, -1, 30, 400, -11, 55, 1203},
                get_random_vector(10),
                get_random_vector(1e3),
                get_random_vector(1e3),
                get_random_vector(1e3)
        };
        int n_test = 1;
        for (auto &test: tests) {
            consistent_tree<int> tree;
            std::set<int> s;

            int n_iter = 1;
            for (auto it: test) {
                tree.insert(it);
                s.insert(it);

                auto res = tree.to_vector();
                REQUIRE(res == set_to_vector(s),
                        "test: " + std::to_string(n_test) +
                        ", iter: " + std::to_string(n_iter));

                n_iter++;
            }
            n_test++;
        }
    }

    void find() {
        std::vector<std::vector<int>> tests = {
                {},
                {-10, 100},
                {1, 2, 3, 4, 5, 6},
                {3, -1, 30, 400, -11, 55, 1203},
                get_random_vector(10),
                get_random_vector(1e3),
                get_random_vector(1e3),
                get_random_vector(1e3)
        };
        int n_test = 1;
        for (auto &test: tests) {
            consistent_tree<int> tree;
            std::set<int> s;

            int n_iter = 1;
            for (auto it: test) {
                tree.insert(it);
                s.insert(it);

                REQUIRE(tree.find(it) == tree.find(it), "case 1");
                REQUIRE(tree.find(it) != tree.end(), "case 2");
                REQUIRE((*tree.find(it)).get() == it, "case 3");

                REQUIRE(tree.find(-it - 1) == tree.end(), "case 4");

                n_iter++;
            }

            auto v = get_random_vector(2 * 1e3, -1e3);
            for (auto it: v) {
                bool set_res = s.find(it) == s.end();
                bool tree_res = tree.find(it) == tree.end();
                REQUIRE(set_res == tree_res, "case 5");
            }

            n_test++;
        }
    }

    void erase() {
        test_case = "erase";
        std::vector<std::vector<int>> tests = {
                {-10, 100},
                {1, 2, 3, 4, 5, 6},
                {3, -1, 30, 400, -11, 55, 1203},
                get_random_vector(10),
                get_random_vector(1e3),
                get_random_vector(1e3),
                get_random_vector(1e3)
        };
        int n_test = 1;
        for (auto &test: tests) {
            consistent_tree<int> tree;

            for (auto it: test) {
                tree.insert(it);
            }

            int n_deleted = 0;
            for (auto it: test) {
                REQUIRE(tree.find(it) != tree.end(), "case 1");

                tree.erase(it);
                n_deleted++;
                REQUIRE(tree.find(it) == tree.end(), "case 2");

                REQUIRE(tree.n_deleted_node == n_deleted, "case 3");
            }
            REQUIRE(tree.empty(), "case 4");


            std::set<int> s;
            tree = consistent_tree<int>();

            for (auto it: test) {
                tree.insert(it);
                s.insert(it);
            }

            auto v = get_random_vector(2 * 1e3, -1e3);

            n_deleted = 0;
            for (auto it: v) {
                bool set_res = s.find(it) == s.end();
                bool tree_res = tree.find(it) == tree.end();
                REQUIRE(set_res == tree_res, "case 5");

                if (s.find(it) != s.end()) {
                    n_deleted++;
                }
                s.erase(it);
                tree.erase(it);
                REQUIRE(tree.n_deleted_node == n_deleted, "case 6");

                set_res = s.find(it) == s.end();
                tree_res = tree.find(it) == tree.end();
                REQUIRE(set_res == tree_res, "case 7");
            }

            n_test++;
        }
    }


    void front() {
        test_case = "front";
        consistent_tree<int> tree;

        auto v1 = get_random_vector(2 * 1e3, -1e3);
        std::set<int> s;
        for (auto it: v1) {
            if (s.begin() != s.end()) {
                REQUIRE(tree.front() == *s.begin(), "case 1");
            }
            tree.insert(it);
            s.insert(it);
            REQUIRE(tree.front() == *s.begin(), "case 2");
        }

        tree.clear();
        for (int i = 1e5; i >= 0; --i) {
            tree.insert(i);
            REQUIRE(tree.front() == i, "case 3");
        }
    }

    void back() {
        test_case = "back";
        consistent_tree<int> tree;

        REQUIRE(tree.begin() == tree.end(), "case 1");

        auto v1 = get_random_vector(1e3);
        std::set<int> s;
        for (auto it: v1) {
            tree.insert(it);
            s.insert(it);

            REQUIRE(tree.back() == *s.rbegin(), "case 2");
        }

        tree.clear();
        for (int i = 0; i < 1e5; ++i) {
            tree.insert(i);
            REQUIRE(tree.back() == i, "case 3");
        }
    }

    void front_iter() {
        test_case = "front_iter";
        consistent_tree<int> tree;

        size_t N = 1e3;
        for (int i = 0; i < N; ++i) {
            tree.insert(i);
        }

        std::vector<typename consistent_tree<int>::iterator> v_it(N);
        for (int i = 0; i < N; ++i) {
            v_it[i] = tree.find(i);
        }

        for (int i = 0; i < N; i++) {
            REQUIRE(tree.front() == i, "case 1");
            tree.erase(i);
            if (i != N - 1) {
                REQUIRE(tree.front() == i + 1, "case 2");
            }
        }

        REQUIRE(tree.empty(), "case 3");
    }

    void back_iter() {
        test_case = "back_iter";
        consistent_tree<int> tree;

        size_t N = 1e3;
        for (int i = 0; i < N; ++i) {
            tree.insert(i);
        }

        std::vector<typename consistent_tree<int>::iterator> v_it(N);
        for (int i = 0; i < N; ++i) {
            v_it[i] = tree.find(i);
        }

        for (int i = (int)N - 1; i >= 0; i--) {
            REQUIRE(tree.back() == i, "case 1");
            tree.erase(i);
            if (i != 0) {
                REQUIRE(tree.back() == i - 1, "case 2");
            }
        }

        REQUIRE(tree.empty(), "case 3");
    }

    void size_and_empty() {
        test_case = "size";
        consistent_tree<int> tree;

        REQUIRE(tree.size() == 0 && tree.empty(), "case 1");

        auto v1 = get_random_vector(1e3);
        std::set<int> s;
        for (auto it: v1) {
            tree.insert(it);
            s.insert(it);

            REQUIRE(tree.size() == s.size(), "case 2");
            REQUIRE(!tree.empty(), "case 3");
        }

        for (int i = (int)v1.size() - 1; i >= 0; i--) {
            tree.erase(v1[i]);
            s.erase(v1[i]);

            REQUIRE(tree.size() == s.size(), "case 4");

            tree.erase(v1[i]);
            REQUIRE(tree.size() == s.size(), "case 5");
            REQUIRE(tree.empty() == s.empty(), "case 6");
        }

        REQUIRE(tree.empty(), "case 7");
    }


    void begin() {
        test_case = "begin";
        consistent_tree<int> tree;

        REQUIRE(tree.begin() == tree.end(), "case 1");

        auto v1 = get_random_vector(2 * 1e3, -1e3);
        std::set<int> s;
        for (auto it: v1) {
            if (s.begin() != s.end()) {
                REQUIRE((*tree.begin()).get() == *s.begin(), "case 2");
            }
            tree.insert(it);
            s.insert(it);
            REQUIRE((*tree.begin()).get() == *s.begin(), "case 3");
        }
    }

    void begin_remove() {
        test_case = "begin_remove";
        consistent_tree<int> tree;

        REQUIRE(tree.begin() == tree.end(), "case 1");

        auto v1 = get_random_vector(2 * 1e3, -1e3);
        std::set<int> s;

        for (auto it: v1) {
            tree.insert(it);
            s.insert(it);
        }

        for (auto it: v1) {
            REQUIRE((*tree.begin()).get() == *s.begin(), "case 2");

            auto begin_value = *s.begin();
            s.erase(begin_value);
            tree.erase(begin_value);

            REQUIRE((*tree.begin()).get() == *s.begin(), "case 3");
        }
    }

    void end() {
        test_case = "end";
        consistent_tree<int> tree;

        REQUIRE(tree.begin() == tree.end(), "case 1");

        auto v1 = get_random_vector(1e3);
        std::set<int> s;
        for (auto it: v1) {
            tree.insert(it);
            s.insert(it);

            bool tree_res, set_res;
            for (int i = 0; i <= 1e3; ++i) {
                tree_res = tree.find(i) == tree.end();
                set_res = s.find(i) == s.end();

                REQUIRE(tree_res == set_res, "case 2");
            }
        }
    }

    void end_remove() {
        test_case = "end_remove";
        consistent_tree<int> tree;

        int value = 1;
        auto real_end = tree.end();

        for (int i = 0; i < 1e5; ++i) {
            REQUIRE(tree.find(value) == tree.end(), "case 1");
            REQUIRE(tree.end() == real_end, "case 2");

            tree.insert(value);

            REQUIRE(tree.find(value) != tree.end(), "case 3");
            REQUIRE(tree.end() == real_end, "case 4");

            tree.erase(value);
        }

        consistent_tree<int> tree1;
        REQUIRE(tree.end() != tree1.end(), "case 5");
    }


    void iterator() {
        test_case = "iterator";
        consistent_tree<int> tree;
        std::set<int> s;
        auto v = get_random_vector(1e3);
        for (int i = 0; i < v.size(); ++i) {
            tree.insert(v[i]);
            s.insert(v[i]);
        }

        auto s_it = s.begin();
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            REQUIRE((*it).get() == *s_it);
            s_it++;
        }
    }

    void iterator_2() {
        test_case = "iterator_2";
        consistent_tree<int> tree;
        auto v = get_random_vector(1e3);
        for (int i = 0; i < v.size(); ++i) {
            tree.insert(v[i]);
        }

        std::vector<typename consistent_tree<int>::iterator> v_it(v.size());
        int i = 0;
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            v_it[i++] = it;
        }

        auto it1 = v_it[0];
        for (i = 1; i < v_it.size(); i++) {
            REQUIRE(v_it[i] == ++it1, "case 1");
        }

        for (auto it : v) {
            tree.erase(it);
            REQUIRE(tree.n_deleted_node == 0, "case 2");
        }

        v_it.clear();
        REQUIRE(tree.n_deleted_node == v.size() - 1, "case 3");
        it1 = consistent_tree<int>::iterator();
        REQUIRE(tree.n_deleted_node == v.size(), "case 4");
    }

    void iterator_3() {
        test_case = "iterator_3";
        consistent_tree<int> tree;
        std::set<int> s;
        auto v = get_random_vector(1e3);
        for (auto &it: v) {
            tree.insert(it);
            s.insert(it);
        }

        std::vector<typename consistent_tree<int>::iterator> v_it(v.size());
        for (int i = 0; i < v.size(); ++i) {
            v_it[i] = tree.find(v[i]);
        }

        for (int i = 0; i < v_it.size(); i++) {
            if (i % 2) {
                tree.erase(v_it[i]);
                s.erase((*v_it[i]).get());

                auto a = tree.find(v[i]);
                auto b = tree.end();
                if (a != b) {
                    REQUIRE(a == b, "case 1");
                }
            }

            auto tv = tree.to_vector();
            auto sv = set_to_vector(s);

            REQUIRE(tv == sv, "case 2");
        }
    }


    void inc_iterator() {
        test_case = "inc_iterator";
        consistent_tree<int> tree;
        tree.insert(1);
        tree.insert(2);
        tree.insert(3);
        tree.insert(4);
        tree.insert(5);

        auto it1 = tree.find(1);
        auto it2 = tree.find(2);
        auto it3 = tree.find(3);
        auto it4 = tree.find(4);
        auto it5 = tree.find(5);

        tree.erase(it2);
        tree.erase(it3);
        tree.erase(it5);

        REQUIRE(++it1 == it4, "case 1");
        REQUIRE(++it2 == it4, "case 2");
        REQUIRE(++it3 == it4, "case 3");
        REQUIRE(++it4 == tree.end(), "case 4");
    }

    void dec_iterator() {
        test_case = "dec_iterator";

        consistent_tree<int> tree;
        auto v = get_random_vector(1e1);
        for (int i = 0; i < v.size(); ++i) {
            tree.insert(v[i]);
        }

        std::vector<typename consistent_tree<int>::iterator> v_it(v.size());
        int i = 0;
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            v_it[i++] = it;
        }

        auto it = tree.end();
        for (i = (int)v_it.size() - 1; i >= 0; i--) {
            REQUIRE(v_it[i] == --it, "case 1");
        }


        tree.clear();
        tree.insert(1);
        tree.insert(2);
        tree.insert(3);
        tree.insert(4);
        tree.insert(5);

        auto it1 = tree.find(1);
        auto it2 = tree.find(2);
        auto it3 = tree.find(3);
        auto it4 = tree.find(4);
        auto it5 = tree.find(5);

        tree.erase(it2);
        tree.erase(it3);
        tree.erase(it5);

        REQUIRE(--it2 == it1, "case 2");
        REQUIRE(--it3 == it1, "case 3");
        REQUIRE(--it5 == it4, "case 4");
        REQUIRE(--it4 == it1, "case 5");
    }


    void destructor() {
        test_case = "destructor";
        auto *receiver1 = new receiver();

        auto *tree1 = new consistent_tree<int>(receiver1);
        delete tree1;

        REQUIRE(receiver1->value == 1, "case 1");


        receiver1->value = 0;
        auto *tree2 = new consistent_tree<int>(receiver1);
        int size = 1e3;
        auto v = get_random_vector(size);
        for (auto it: v) {
            tree2->insert(it);
        }
        delete tree2;

        REQUIRE(receiver1->value == size + 1, "case 2");


        receiver1->value = 0;
        auto *tree3 = new consistent_tree<int>(receiver1);

        size = 1e3;
        auto v3 = get_random_vector(size);
        for (int i = 0; i < size; i++) {
            tree3->insert(v3[i]);
            if (i % 2) {
                tree3->erase(v3[i]);
            }
        }
        delete tree3;

        REQUIRE(receiver1->value == size + 1, "case 3");

        delete receiver1;
    }

public:
    void run() {
        std::cout << "-------tree_test.h-------\n";

        to_vector();

        insert();
        find();
        erase();

        front();
        front_iter();
        back();
        back_iter();

        size_and_empty();

        begin();
        begin_remove();
        end();
        end_remove();

        iterator();
        iterator_2();
        iterator_3();

        inc_iterator();
        dec_iterator();

        destructor();

        std::cout << test_counter - fail_counter << " TEST PASSED\n";
        std::cout << fail_counter << " TEST FAILED\n";
        std::cout << "-------------------------\n\n";
    }
};
/** tree_test.h */

int main() {
    tree_test().run();

    return 0;
}
