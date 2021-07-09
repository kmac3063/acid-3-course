#pragma once

#include "iostream"
#include "vector"

#include "utils.h"
#include "consistent_linked_list.h"

const int N_TEST = 100;

string test_case = "NULL";

void REQUIRE(bool b) {
    if (!b) {
        throw runtime_error("Fail. Test: " + test_case);
    }
}

void push_back() {
    test_case = "push_back";
    consistent_linked_list<int> list;
    vector<int> v = {};
    for (int i = 0; i < N_TEST; i++) {
        int rnd_number = rand(-1e3, 1e3);

        v.push_back(rnd_number);
        list.push_back(rnd_number);
        REQUIRE(list.to_vector() == v);
    }
}

void push_front() {
    test_case = "push_front";
    consistent_linked_list<int> list;
    vector<int> v(N_TEST);
    for (int i = 0; i < N_TEST; i++) {
        int rnd_number = rand(-1e3, 1e3);

        v[N_TEST - i - 1] = rnd_number;
        list.push_front(rnd_number);


        bool equal = 1;
        auto list_vector = list.to_vector();
        int index = 0;

        REQUIRE(list_vector.size() == i + 1);

        for (int j = N_TEST - i - 1; j < N_TEST; j++) {
            if (v[j] != list_vector[index++]) {
                equal = 0;
                break;
            }
        }
        REQUIRE(equal);
    }
}

void pop_first() {
    test_case = "pop_first";
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

void pop_last() {
    test_case = "pop_last";
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

void front() {
    test_case = "front";
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

void back() {
    test_case = "back";
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

void begin() {

}

void end() {

}

void empty_from_vector() {
    consistent_linked_list<int> list_from_vector(get_vec<int>({}));
    REQUIRE(list_from_vector.empty());
    list_from_vector.push_back(1);
    REQUIRE(!list_from_vector.empty());

    consistent_linked_list<int> list_from_vector_not_empty(get_vec({1, 2, 3}));
    REQUIRE(!list_from_vector_not_empty.empty());
}

void empty_push_and_pop() {
    consistent_linked_list<int> list;
    REQUIRE(list.empty());

    for (int i = 0; i < N_TEST; ++i) {
        list.push_back(i);
        REQUIRE(!list.empty());

        list.pop_first();
        REQUIRE(list.empty());
    }
}

void empty_push_then_pop() {
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

void size() {
    test_case = "size";
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

void size_random() {
    test_case = "size_random";
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

void erase() {
    test_case = "erase";
    consistent_linked_list<int> list;

    fill_range(list, 0, N_TEST - 1);

    for (int i = 0; i < N_TEST; ++i) {
        REQUIRE(list.contain(i));
        list.erase(i);
        REQUIRE(!list.contain(i));
    }
    REQUIRE(list.n_deleted_node == N_TEST);
}

void find() {
    test_case = "find";
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

void contain() {
    test_case = "contain";
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

void to_vector() {
    test_case = "to_vector";
    consistent_linked_list<int> list;
    vector<int> v;

    for (int i = 0; i < N_TEST; ++i) {
        int rnd_number = rand(-1e3, 1e3);

        list.push_back(rnd_number);
        v.push_back(rnd_number);
    }

    REQUIRE(v == list.to_vector());
}

void start_func_tests() {
    push_back();
    push_front();
    pop_first();
    pop_last();
    front();
    back();
    begin();
    end();
    empty_from_vector();
    empty_push_and_pop();
    empty_push_then_pop();
    size();
    size_random();
    erase();
    contain();
    to_vector();
    find();

    std::vector<int> v;

    std::cout << "Function tests passed. Nice!";
}
