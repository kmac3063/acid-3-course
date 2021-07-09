#pragma once

#include "iostream"
#include "vector"
#include <thread>

#include "utils.h"
#include "consistent_linked_list.h"

namespace threads_with_lock_list_tests {
    using namespace std;

    const int N_TEST = 100;
    int N_THREADS = 4;

    string test_case = "NULL";

    void REQUIRE(bool b) {
        if (!b) {
            throw runtime_error("Fail. Test: " + test_case);
        }
    }

    void REQUIRE(int a, int b) {
        if (a != b) {
            cout << "Found: " + to_string(a) +". Expected: " + to_string(b) << endl;
            throw runtime_error("Fail. Test: " + test_case);
        }
    }

    void push_1() {
        test_case = "push_1";

        consistent_linked_list<int> list;

        vector<thread> vt(N_THREADS);
        for (int i = 0; i < N_THREADS; ++i) {
            vt[i] = thread([&]() -> void {
                for (int j = 0; j < N_TEST; ++j) {
                    list.push_back(1);
                }
            });
        }

        for (int i = 0; i < N_THREADS; ++i) {
            vt[i].join();
        }


        REQUIRE(list.size(), N_THREADS * N_TEST);
    }

    void push_2() {
        test_case = "push_2";
        consistent_linked_list<int> list;

        vector<int> ans;
        vector<int> t;

        int N_ONE = N_TEST;
        int N_TWO = 1;
        int N_THREE = N_TEST;

        t = vector(N_ONE, 1);
        ans.insert(ans.begin(), t.begin(), t.end());

        t = vector(N_TWO, 2);
        ans.insert(ans.end(), t.begin(), t.end());

        t = vector(N_THREE, 3);
        ans.insert(ans.end(), t.begin(), t.end());

        for (int i = 0; i < N_TWO; ++i) {
            list.push_back(2);
        }

        thread thread1([&]() -> void {
            for (int i = 0; i < N_ONE; ++i) {
                list.push_front(1);
            }
        });

        thread thread2([&]() -> void {
            for (int i = 0; i < N_THREE; ++i) {
                list.push_back(3);
            }
        });

        thread1.join();
        thread2.join();

        REQUIRE(list.to_vector() == ans);
    }

    void pop_first() {
        test_case = "pop_first";

        vector<int> t(N_THREADS * N_TEST, 1);

        consistent_linked_list<int> list(t);

        vector<thread> vt(N_THREADS);
        for (int i = 0; i < N_THREADS; ++i) {
            vt[i] = thread([&]() -> void {
                for (int j = 0; j < N_TEST; ++j) {
                    list.pop_first();
                }
            });
        }

        for (int i = 0; i < N_THREADS; ++i) {
            vt[i].join();
        }

        REQUIRE(list.size(), 0);
    }

    void pop_last() {
        test_case = "pop_last";

        vector<int> t(N_THREADS * N_TEST, 1);

        consistent_linked_list<int> list(t);

        vector<thread> vt(N_THREADS);
        for (int i = 0; i < N_THREADS; ++i) {
            vt[i] = thread([&]() -> void {
                for (int j = 0; j < N_TEST; ++j) {
                    list.pop_last();
                }
            });
        }

        for (int i = 0; i < N_THREADS; ++i) {
            vt[i].join();
        }

        REQUIRE(list.size(), 0);
    }

    void pop_first_and_last() {
        test_case = "pop_first_and_last";

        vector<int> t(2 * N_TEST, 1);
        consistent_linked_list<int> list(t);

        vector<thread> vt(N_THREADS);
        vt[0] = thread([&]() -> void {
            for (int j = 0; j < N_TEST; ++j) {
                list.pop_first();
            }
        });
        vt[1] = thread([&]() -> void {
            for (int j = 0; j < N_TEST; ++j) {
                list.pop_last();
            }
        });


        for (int i = 0; i < 2; ++i) {
            vt[i].join();
        }

        REQUIRE(list.size(), 0);
    }

    void erase() {
        test_case = "erase";

        vector<int> numbers(N_THREADS * N_TEST);
        for (int i = 0; i < numbers.size(); ++i) {
            numbers[i] = i;
        }
        consistent_linked_list<int> list(numbers);

        random_shuffle(numbers.begin(), numbers.end());

        vector<thread> vt(N_THREADS);
        mutex m;
        for (int i = 0; i < N_THREADS; ++i) {
            vt[i] = thread([&, i]() -> void {
                for (int j = i * N_TEST; j < (i + 1) * N_TEST; ++j) {
                    list.erase(j);
                }
            });
        }

        for (int i = 0; i < N_THREADS; ++i) {
            vt[i].join();
        }

        REQUIRE(list.size(), 0);
    }

    void start() {
        push_1();
        push_2();
        pop_first();
        pop_last();
        pop_first_and_last();
        erase();

        std::cout << "Threads tests with lock list passed. Nice!" << endl;
    }
}