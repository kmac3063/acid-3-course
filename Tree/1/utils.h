#pragma once

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <random>

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