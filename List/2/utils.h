#pragma once

#include <vector>

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
