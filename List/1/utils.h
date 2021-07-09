#pragma once

#include <vector>
#include <set>

#include "consistent_linked_list.h"

template<typename T>
std::vector<T> get_vec(const std::initializer_list<T> &list) {
    return std::vector<T>(list);
}

template<typename T>
void fill_range(consistent_linked_list<T> &list, const T &left, const T &right) {
    for (int i = left; i <= right; ++i) {
        list.push_back(i);
    }
}

template<typename T>
T rand(const T &left, const T &right) {
    return (int) left + rand() % ((int) right - (int) left + 1);
}

template<typename T>
void fill_random(consistent_linked_list<T> &list, const int &n, const T &left, const T &right) {
    for (int i = 0; i < n; ++i) {
        T rnd_number = rand(left, right);
        list.push_back(rnd_number);
    }
}

template<typename T>
std::string v_to_str(const vector<T>& v) {
    string res = "[";
    for (int i = 0; i < v.size(); i++) {
        res += v[i];
        if (i != v.size() - 1) {
            res += ", ";
        }
    }
    return res + ']';
}

template<typename T>
std::set<T> v_to_set(const vector<T>& v) {
    return std::set(v.begin(), v.end());
}

template<typename T>
std::vector<T> set_to_vector(std::set<T> s) {
    return std::vector(s.begin(), s.end());
}

template<typename T>
get_fake_tree(vector)

