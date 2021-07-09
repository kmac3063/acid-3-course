#pragma once

#include <iostream>
#include <vector>
#include <exception>
#include <mutex>
#include <thread>

class consistent_linked_list_exception : std::exception {
public:
    std::string reason;

    consistent_linked_list_exception(const std::string &reason_) {
        reason = reason_;
    }

    char *what() {
        return const_cast<char *>(reason.c_str());
    }
};

template<typename T>
class consistent_linked_list {
private:
    class Node {
    public:
        Node(consistent_linked_list<T> *base_list_, const T &t) :
                base_list(base_list_), value(t) {}

        consistent_linked_list<T> *base_list;
        T value;
        Node *prev = nullptr;
        Node *next = nullptr;

        bool is_deleted = false;
        int ref_count = 0;

        void add_ref_count(const int &value_) {
            if (this == base_list->END_NODE) {
                return;
            }

            ref_count += value_;
            if (ref_count <= 0) {
                base_list->n_deleted_node++;
//                cout << "It's all, we deleted :( value = " << value << endl;
                delete this;
            }
        }
    };

    std::recursive_mutex m;

    Node *END_NODE;

    Node *first;
    Node *last;

    size_t list_size = 0;

    Node *create_new_node(const T &value) {
        return new Node(this, value);
    }

    void remove_node(Node *node) {
        if (node->is_deleted) return;

        node->is_deleted = 1;

        Node *prev = node->prev;
        Node *next = node->next;

        prev->next = next;
        next->prev = prev;

        node->add_ref_count(-2);

        if (first == last) {
            first = last = END_NODE;
        } else if (node == first) {
            first = next;
        } else if (node == last) {
            last = prev;
        }


        list_size--;
    }

public:
    size_t n_deleted_node = 0;

    class consistent_iterator;

    consistent_linked_list() {
        END_NODE = new Node(this, 0);
        END_NODE->next = END_NODE;
        END_NODE->prev = END_NODE;
        first = last = END_NODE;
    };

    consistent_linked_list(const std::vector<T> &v) : consistent_linked_list() {
        for (auto &el : v) {
            push_back(el);
        }
    }

    ~consistent_linked_list() {
        for (auto it = begin(); it != end(); it++) {
            erase(it);
        }
        delete END_NODE;
    }

    void push_front(const T &value) {
        Node *new_node = create_new_node(value);

        m.lock();
        new_node->prev = END_NODE;
        END_NODE->next = new_node;

        new_node->next = first;
        first->prev = new_node;

        new_node->add_ref_count(2);

        first = new_node;
        if (last == END_NODE) {
            last = first;
        }

        list_size++;
        m.unlock();
    }

    void push_back(const T &value) {
        Node *new_node = create_new_node(value);

        m.lock();
        new_node->prev = last;
        last->next = new_node;

        new_node->next = END_NODE;
        END_NODE->prev = new_node;

        new_node->add_ref_count(2);

        last = new_node;
        if (first == END_NODE) {
            first = last;
        }

        list_size++;
        m.unlock();
    }

    void pop_first() {
        m.lock();
        remove_node(first);
        m.unlock();
    }

    void pop_last() {
        m.lock();
        remove_node(last);
        m.unlock();
    }

    T front() {
        m.lock();
        if (list_size == 0) {
            m.unlock();
            throw consistent_linked_list_exception("List size is 0.");
        }
        T res = first->value;
        m.unlock();
        return res;
    }

    T back() {
        m.lock();
        if (list_size == 0) {
            m.unlock();
            throw consistent_linked_list_exception("List size is 0.");
        }
        T res = last->value;
        m.unlock();
        return res;
    }

    consistent_iterator begin() {
        m.lock();
        auto res = consistent_iterator(first);
        m.unlock();
        return res;
    }

    consistent_iterator end() {
        m.lock();
        auto res = consistent_iterator(END_NODE);
        m.unlock();
        return res;
    }

    bool empty() {
        m.lock();
        int res = list_size;
        m.unlock();
        return res == 0;
    }

    size_t size() {
        m.lock();
        int res = list_size;
        m.unlock();
        return res;
    }

    void erase(consistent_iterator t) {
        m.lock();
        Node *node = t.get_node();
        if (node == END_NODE) {
            m.unlock();
            throw consistent_linked_list_exception("Deleted end iterator.");
        }
        remove_node(node);
        m.unlock();
    }

    void erase(const T &value) {
        m.lock();
        consistent_iterator it = begin();
        for (; it != end(); it++) {
            if (*it == value) {
                break;
            }
        }
        if (it != end()) {
            erase(it);
        }
        m.unlock();
    }

    consistent_iterator find(const T &value) {
        m.lock();
        for (consistent_iterator it = begin(); it != end(); it++) {
            if (*it == value) {
                auto res = it;
                m.unlock();
                return res;
            }
        }
        auto res = end();
        m.unlock();
        return res;
    }

    bool contain(const T &value) {
        m.lock();
        consistent_iterator it = begin();
        consistent_iterator end_it = end();
        while (it != end_it) {
            if (value == *it) {
                m.unlock();
                return true;
            }
            it++;
        }
        m.unlock();
        return false;
    }

    void shrink_to_fit() {
        m.lock();
        consistent_linked_list list;
        consistent_iterator it = begin();
        consistent_iterator end_it = end();
        while (it != end_it) {
            list.push_back(*it);
        }

        while (first != END_NODE) {
            Node *next = first->next;
            delete first;
            first = next;
        }

        m.unlock();
        *this = list;
    }

    void print() {
        m.lock();
        std::string offset_space(' ', 3);
        std::cout << "{ size = " << list_size << std::endl;
        consistent_iterator it = begin();
        for (size_t i = 0; i < list_size; i++) {
            std::cout << offset_space <<
                 "[value = " << *it <<
                 ", ref_count = " << it->get_node()->ref_count <<
                 "]";
            it++;
            if (i != list_size - 1) {
                std::cout << ", ";
            }
            std::cout << '\n';
        }
        std::cout << "}\n";
        m.unlock();
    }

    std::vector<T> to_vector() {
        m.lock();
        std::vector<T> v(list_size);
        int i = 0;
        for (auto it = begin(); it != end(); it++) {
            v[i++] = *it;
        }
        m.unlock();
        return v;
    }

    class consistent_iterator {
    private:
        std::recursive_mutex &m;
        Node *node = nullptr;

        Node *get_not_deleted_prev(Node *node_) {
            Node *prev = node_->prev;
            Node *end_node = node_->base_list->END_NODE;
            while (prev->is_deleted && prev != end_node) {
                prev = prev->prev;
            }
            return prev;
        }

        Node *get_not_deleted_next(Node *node_) {
            Node *next = node_->next;
            Node *end_node = node_->base_list->END_NODE;
            while (next->is_deleted && next != end_node) {
                next = next->next;
            }
            return next;
        }

    public:
        consistent_iterator(Node *node_) : m(node_->base_list->m) {
            node = node_;
            node->add_ref_count(1);
        }

        consistent_iterator(const consistent_iterator &original) :
                consistent_iterator(original.node) {}

        ~consistent_iterator() {
            node->add_ref_count(-1);
        }

        T operator*() {
            return node->value;
        }

        Node *get_node() {
            return node;
        }

        // prefix++
        consistent_iterator operator++() {
            m.lock();
            if (node == node->base_list->END_NODE) {
                m.unlock();
                throw consistent_linked_list_exception("No more element.");
            }

            Node *next = get_not_deleted_next(node);

            node->add_ref_count(-1);
            node = next;
            node->add_ref_count(1);

            auto res = consistent_iterator(node);
            m.unlock();
            return res;
        }

        // postfix++
        consistent_iterator operator++(int) {
            m.lock();
            if (node == node->base_list->END_NODE) {
                m.unlock();
                throw consistent_linked_list_exception("No more element.");
            }

            Node *next = get_not_deleted_next(node);
            consistent_iterator temp = consistent_iterator(node);

            node->add_ref_count(-1);
            node = next;
            node->add_ref_count(1);

            m.unlock();
            return temp;
        }

        // prefix--
        consistent_iterator operator--() {
            m.lock();
            Node *prev = get_not_deleted_prev(node);

            if (prev == node->base_list->END_NODE) {
                m.unlock();
                throw consistent_linked_list_exception("It's first element.");
            }

            node->add_ref_count(-1);
            node = prev;
            prev->add_ref_count(1);

            auto res = consistent_iterator(node);
            m.unlock();
            return res;
        }

        // postfix--
        consistent_iterator operator--(int) {
            m.lock();
            Node *prev = get_not_deleted_prev(node);

            if (prev == node->base_list->END_NODE) {
                m.unlock();
                throw consistent_linked_list_exception("It's first element.");
            }

            consistent_iterator temp = consistent_iterator(node);

            node->add_ref_count(-1);
            node = prev;
            node->add_ref_count(1);

            m.unlock();
            return temp;
        }

        bool operator!=(consistent_iterator rhs) const {
            m.lock();
            bool b = &(*this->node) != &(*rhs.node);
            m.unlock();
            return b;
        }

        bool operator==(consistent_iterator rhs) const {
            m.lock();
            bool b = &(*this->node) == &(*rhs.node);
            m.unlock();
            return b;
        }

        void erase() {
            m.lock();
            if (node->is_deleted)
                m.unlock();
            return;

            node->base_list->erase(*this);
            m.unlock();
        }

        static consistent_iterator next(consistent_iterator it) {
            if (it.get_node() == it.get_node()->base_list->END_NODE) {
                throw consistent_linked_list_exception("No more elements");
            }

            Node *next = it.get_not_deleted_next(it.get_node());

            it.get_node()->add_ref_count(-1);
            next->add_ref_count(1);
            return consistent_iterator(next);
        }

        static consistent_iterator prev(consistent_iterator it) {
            Node *prev = it.get_not_deleted_prev(it.get_node());

            if (prev == it.get_node()->base_list->END_NODE) {
                throw consistent_linked_list_exception("It's first element.");
            }


            it.get_node()->add_ref_count(-1);
            prev->add_ref_count(1);
            return consistent_iterator(prev);
        }
    };

};
