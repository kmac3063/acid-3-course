#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

#include "catch.hpp"

#include "consistent_linked_list.h"

using namespace std;

const int N_TEST = 100;


template<typename T>
vector<T> get_vec(const initializer_list<T> &list) {
    return vector<T>(list);
}

template<typename T>
void fill_range(consistent_linked_list<T> &list, const T &left, const T &right) {
    for (int i = left; i <= right; ++i) {
        list.push_back(i);
    }
}

template<typename T>
T rand(const T &left, const T &right) {
    return (long long) left + rand() % ((long long) right - (long long) left + 1);
}

template<typename T>
void fill_random(consistent_linked_list<T> &list, const int &n, const T &left, const T &right) {
    for (int i = 0; i < n; ++i) {
        list.push_back(rand(left, right));
    }
}

TEST_CASE("push_back") {
    consistent_linked_list<int> list;
    vector<int> v = {};
    for (int i = 0; i < N_TEST; i++) {
        int rnd_number = rand(-1e3, 1e3);

        v.push_back(rnd_number);
        list.push_back(rnd_number);
        REQUIRE(list.to_vector() == v);
    }
}

TEST_CASE("push_front") {
    consistent_linked_list<int> list;
    vector<int> v(N_TEST);
    for (int i = 0; i < N_TEST; i++) {
        int rnd_number = rand(-1e3, 1e3);

        v[N_TEST - i - 1] = rnd_number;
        list.push_front(rnd_number);


        bool equal = 1;
        auto list_vector = list.to_vector();
        int index = 0;


        REQUIRE(list_vector.size() == v.size());

        for (int j = N_TEST - i - 1; j < N_TEST; j++) {
            if (v[j] != list_vector[index++]) {
                equal = 0;
                break;
            }
        }
        REQUIRE(equal);
    }
}

TEST_CASE("pop_first") {
    consistent_linked_list<int> list;
    fill_random(list, N_TEST, (int) -1e3, (int) 1e3);
    auto v = list.to_vector();

    for (int i = 0; i < N_TEST; ++i) {
        list.pop_first();

        auto list_vector = list.to_vector();
        bool equal = 1;

        for (int j = i + 1; j < N_TEST; j++) {
            if (v[j] != list_vector[j - i - 1]) {
                equal = 0;
                break;
            }
        }
        REQUIRE(equal);
    }
}

TEST_CASE("pop_last") {
    consistent_linked_list<int> list;
    fill_random(list, N_TEST, 1, 100);
    auto v = list.to_vector();

    for (int i = 0; i < N_TEST; ++i) {
        list.pop_last();

        auto list_vector = list.to_vector();
        bool equal = 1;

        for (int j = 0; j < N_TEST - i - 1; j++) {
            if (v[j] != list_vector[j]) {
                equal = 0;
                break;
            }
        }
        REQUIRE(equal);
    }
}

TEST_CASE("front") {
    consistent_linked_list<int> list;
    const int BACK_NUMBER = 0;
    list.push_back(BACK_NUMBER);
    for (int i = 0; i < N_TEST - 1; ++i) {
        int rnd_number = rand(-1e3, 1e3);
        list.push_front(rnd_number);

        REQUIRE(list.front() == rnd_number);
        REQUIRE(list.back() == BACK_NUMBER);
    }
}

TEST_CASE("back") {
    consistent_linked_list<int> list;
    const int FRONT_NUMBER = 0;
    list.push_front(FRONT_NUMBER);
    for (int i = 0; i < N_TEST - 1; ++i) {
        int rnd_number = rand(-1e3, 1e3);
        list.push_back(rnd_number);

        REQUIRE(list.front() == FRONT_NUMBER);
        REQUIRE(list.back() == rnd_number);
    }
}

TEST_CASE("begin") {

}

TEST_CASE("end") {

}

TEST_CASE("empty from vector") {
    consistent_linked_list<int> list_from_vector(get_vec<int>({}));
    REQUIRE(list_from_vector.empty());
    list_from_vector.push_back(1);
    REQUIRE(!list_from_vector.empty());

    consistent_linked_list<int> list_from_vector_not_empty(get_vec({1, 2, 3}));
    REQUIRE(!list_from_vector_not_empty.empty());
}

TEST_CASE("empty push and pop") {
    consistent_linked_list<int> list;
    REQUIRE(list.empty());

    for (int i = 0; i < N_TEST; ++i) {
        list.push_back(i);
        REQUIRE(!list.empty());

        list.pop_first();
        REQUIRE(list.empty());
    }
}

TEST_CASE("empty push, then pop") {
    consistent_linked_list<int> list;
    for (int i = 0; i < N_TEST; ++i) {
        list.push_back(i);
        REQUIRE(!list.empty());
    }

    for (int i = 0; i < N_TEST; ++i) {
        REQUIRE(!list.empty());
        list.pop_last();
    }
    REQUIRE(list.empty());
}

TEST_CASE("size") {
    consistent_linked_list<int> list;
    REQUIRE(list.size() == 0);
    for (int i = 0; i < N_TEST; ++i) {
        list.push_back(0);
        REQUIRE(list.size() == i + 1);
    }

    for (int i = 0; i < N_TEST; ++i) {
        list.pop_last();
        REQUIRE(list.size() == N_TEST - i - 1);
    }
}

TEST_CASE("size_random") {
    consistent_linked_list<int> list;
    REQUIRE(list.size() == 0);
    int real_size = 0;
    for (int i = 0; i < N_TEST; ++i) {
        bool rnd = rand(0, 1);
        if (rnd || real_size == 0) {
            list.push_back(0);
            real_size++;
        } else {
            list.pop_last();
            real_size--;
        }

        REQUIRE(list.size() == real_size);
    }
}

TEST_CASE("erase") {
    consistent_linked_list<int> list;

    fill_range(list, 0, N_TEST - 1);

    for (int i = 0; i < N_TEST; ++i) {
        REQUIRE(list.contain(i));
        list.erase(i);
        REQUIRE(!list.contain(i));
    }
}

TEST_CASE("find") {
    consistent_linked_list<int> list;
    for (int i = 0; i < N_TEST; ++i) {
        REQUIRE(list.find(i) == list.end());
    }

    fill_range(list, 0, N_TEST - 1);

    for (int i = 0; i < N_TEST; ++i) {
        auto it = list.find(i);
        REQUIRE(it != list.end());
        REQUIRE(it.get_node()->value == i);
        REQUIRE(*it == i);
    }

    for (int i = 0; i < N_TEST; ++i) {
        REQUIRE(!list.contain(-i - 1));
    }
}

TEST_CASE("contain") {
    consistent_linked_list<int> list;
    for (int i = 0; i < N_TEST; ++i) {
        REQUIRE(!list.contain(i));
    }

    fill_range(list, 0, N_TEST - 1);

    for (int i = 0; i < N_TEST; ++i) {
        REQUIRE(list.contain(i));
    }

    for (int i = 0; i < N_TEST; ++i) {
        REQUIRE(!list.contain(-i - 1));
    }
}

TEST_CASE("to_vector") {
    consistent_linked_list<int> list;
    vector<int> v;

    for (int i = 0; i < N_TEST; ++i) {
        int rnd_number = rand(-1e3, 1e3);

        list.push_back(rnd_number);
        v.push_back(rnd_number);
    }

    REQUIRE(v == list.to_vector());
}

