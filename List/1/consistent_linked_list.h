#pragma once

#include <iostream>
#include <vector>
#include <exception>

using namespace std;

class consistent_linked_list_exception : exception {
public:
    string reason;

    consistent_linked_list_exception(const string &reason_) {
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

    consistent_linked_list(const vector<T> &v) : consistent_linked_list() {
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
    }

    void push_back(const T &value) {
        Node *new_node = create_new_node(value);

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
    }

    void pop_first() {
        remove_node(first);
    }

    void pop_last() {
        remove_node(last);
    }

    T front() {
        if (list_size == 0) {
            throw consistent_linked_list_exception("List size is 0.");
        }
        return first->value;
    }

    T back() {
        if (list_size == 0) {
            throw consistent_linked_list_exception("List size is 0.");
        }
        return last->value;
    }

    consistent_iterator begin() {
        return consistent_iterator(first);
    }

    consistent_iterator end() {
        return consistent_iterator(END_NODE);
    }

    bool empty() {
        return list_size == 0;
    }

    size_t size() {
        return list_size;
    }

    void erase(consistent_iterator t) {
        Node *node = t.get_node();
        if (node == END_NODE) {
            throw consistent_linked_list_exception("Deleted end iterator.");
        }
        remove_node(node);
    }

    void erase(const T &value) {
        erase(find(value));
    }

    consistent_iterator find(const T &value) {
        for (consistent_iterator it = begin(); it != end(); it++) {
            if (*it == value) {
                return it;
            }
        }
        return end();
    }

    bool contain(const T &value) {
        consistent_iterator it = begin();
        consistent_iterator end_it = end();
        while (it != end_it) {
            if (value == *it) {
                return true;
            }
            it++;
        }
        return false;
    }

    void shrink_to_fit() {
        consistent_linked_list list;
        consistent_iterator it = begin();
        consistent_iterator end_it = end();
        while (it != end_it) {
            list.push_back(*it);
        }

        while (first != END_NODE) {
            Node* next = first->next;
            delete first;
            first = next;
        }

        *this = list;
    }

    void print() {
        string offset_space = string(' ', 3);
        cout << "{ size = " << list_size << endl;
        consistent_iterator it = begin();
        for (size_t i = 0; i < list_size; i++) {
            cout << offset_space <<
                 "[value = " << *it <<
                 ", ref_count = " << it->get_node()->ref_count <<
                 "]";
            it++;
            if (i != list_size - 1) {
                cout << ", ";
            }
            cout << '\n';
        }
        cout << "}\n";
    }

    vector<T> to_vector() {
        vector<T> v(list_size);
        int i = 0;
        for (auto it = begin(); it != end(); it++) {
            v[i++] = *it;
        }
        return v;
    }

    class consistent_iterator {
    private:
        Node *node = nullptr;

        Node* get_not_deleted_prev(Node* node_) {
            Node* prev = node_->prev;
            Node* end_node = node_->base_list->END_NODE;
            while (prev->is_deleted && prev != end_node) {
                prev = prev->prev;
            }
            return prev;
        }

        Node* get_not_deleted_next(Node* node_) {
            Node* next = node_->next;
            Node* end_node = node_->base_list->END_NODE;
            while (next->is_deleted && next != end_node) {
                next = next->next;
            }
            return next;
        }
    public:
        consistent_iterator(Node *node_) {
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
            if (node == node->base_list->END_NODE) {
                throw consistent_linked_list_exception("No more element.");
            }

            Node* next = get_not_deleted_next(node);

            node->add_ref_count(-1);
            node = next;
            node->add_ref_count(1);

            return consistent_iterator(node);
        }

        // postfix++
        consistent_iterator operator++(int) {
            if (node == node->base_list->END_NODE) {
                throw consistent_linked_list_exception("No more element.");
            }

            Node* next = get_not_deleted_next(node);
            consistent_iterator temp = consistent_iterator(node);

            node->add_ref_count(-1);
            node = next;
            node->add_ref_count(1);

            return temp;
        }

        // prefix--
        consistent_iterator operator--() {
            Node* prev = get_not_deleted_prev(node);

            if (prev == node->base_list->END_NODE) {
                throw consistent_linked_list_exception("It's first element.");
            }

            node->add_ref_count(-1);
            node = prev;
            prev->add_ref_count(1);

            return consistent_iterator(node);
        }

        // postfix--
        consistent_iterator operator--(int) {
            Node* prev = get_not_deleted_prev(node);

            if (prev == node->base_list->END_NODE) {
                throw consistent_linked_list_exception("It's first element.");
            }

            consistent_iterator temp = consistent_iterator(node);

            node->add_ref_count(-1);
            node = prev;
            node->add_ref_count(1);

            return temp;
        }

        bool operator!=(consistent_iterator rhs) const {
            return &(*this->node) != &(*rhs.node);
        }

        bool operator==(consistent_iterator rhs) const {
            return &(*this->node) == &(*rhs.node);
        }

        void erase() {
            if (node->is_deleted)
                return;

            node->base_list->erase(*this);
        }

        static consistent_iterator next(consistent_iterator it) {
            if (it.get_node() == it.get_node()->base_list->END_NODE) {
                throw consistent_linked_list_exception("No more elements");
            }

            Node* next = it.get_not_deleted_next(it.get_node());

            it.get_node()->add_ref_count(-1);
            next->add_ref_count(1);
            return consistent_iterator(next);
        }

        static consistent_iterator prev(consistent_iterator it) {
            Node* prev = it.get_not_deleted_prev(it.get_node());

            if (prev == it.get_node()->base_list->END_NODE) {
                throw consistent_linked_list_exception("It's first element.");
            }


            it.get_node()->add_ref_count(-1);
            prev->add_ref_count(1);
            return consistent_iterator(prev);
        }
    };

};
