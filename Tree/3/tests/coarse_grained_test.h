#pragma once

#include "../consistent_tree.h"
#include <thread>
#include <vector>

class coarse_grained_test {
private:
    std::string test_case;
    size_t test_counter = 0;
    size_t fail_counter = 0;

    size_t n_threads = 0;

    void REQUIRE(bool result, const std::string &reason = "") {
        test_counter++;
        if (!result) {
            fail_counter++;
            fail_printer::print("coarse_grained_test.h", test_case, reason);
        } else {
            /*
            std::string s = "[test_case: " + test_case;
            if (!reason.empty()) {
                s += ", reason: " + reason;
            }
            s += "] PASSSED";
            std::cout << s << std::endl;
            */
        }
    }

public:
    coarse_grained_test(size_t n_treads_ = 1) : n_threads(n_treads_) {}

    void insert_one_number() {
        test_case = "insert_one_number";

        consistent_tree<int> tree;

        std::vector<std::thread> vt(n_threads);
        int n_numbers = 1e3;

        for (int i = 0; i < vt.size(); ++i) {
            vt[i] = std::thread([&]() -> void {
                for (int j = 0; j < n_numbers; ++j) {
                    tree.insert(1);
                }
            });
        }

        for (int i = 0; i < n_threads; ++i) {
            vt[i].join();
        }


        REQUIRE(tree.size() == 1, "case 1");
        REQUIRE(tree.front() == 1, "case 2");
    }

    void insert_same_numbers() {
        test_case = "insert_same_numbers";

        consistent_tree<int> tree;

        std::vector<std::thread> vt(n_threads);
        int n_numbers = 1e3;

        for (int i = 0; i < vt.size(); ++i) {
            vt[i] = std::thread([&]() -> void {
                for (int j = 0; j < n_numbers; ++j) {
                    tree.insert(j);
                }
            });
        }

        for (int i = 0; i < n_threads; ++i) {
            vt[i].join();
        }


        REQUIRE(tree.size() == n_numbers, "case 1");
        for (int i = 0; i < n_numbers; ++i) {
            REQUIRE(tree.find(i) != tree.end(), "case 2");
        }
    }

    void insert_different_numbers() {
        test_case = "insert_different_numbers";

        consistent_tree<int> tree;

        std::vector<std::thread> vt(n_threads);
        int n_numbers = 1e3;

        for (int i = 0; i < vt.size(); ++i) {
            vt[i] = std::thread([&](int start) -> void {
                for (int j = start; j < start + n_numbers; ++j) {
                    tree.insert(j);
                }
            }, n_numbers * i);
        }

        for (int i = 0; i < n_threads; ++i) {
            vt[i].join();
        }


        REQUIRE(tree.size() == n_threads * n_numbers, "case 1");
        for (int i = 0; i < n_threads * n_numbers; ++i) {
            REQUIRE(tree.find(i) != tree.end(), "case 2");
        }
    }

    void erase_same_numbers() {
        test_case = "erase_same_numbers";

        consistent_tree<int> tree;

        int n_numbers = 1e5;
        for (int i = 0; i < n_numbers; ++i) {
            tree.insert(i);
        }

        std::vector<std::thread> vt(n_threads);

        for (int i = 0; i < vt.size(); ++i) {
            vt[i] = std::thread([&]() -> void {
                for (int j = 0; j < n_numbers; ++j) {
                    tree.erase(j);
                }
            });
        }

        for (int i = 0; i < n_threads; ++i) {
            vt[i].join();
        }

        REQUIRE(tree.empty(), "case 1");
    }

    void erase_different_numbers() {
        test_case = "erase_different_numbers";

        consistent_tree<int> tree;

        int n_numbers = 1e4;
        for (int i = 0; i < n_threads * n_numbers; ++i) {
            tree.insert(i);
        }

        std::vector<std::thread> vt(n_threads);

        for (int i = 0; i < vt.size(); ++i) {
            vt[i] = std::thread([&](int start) -> void {
                for (int j = start; j < start + n_numbers; ++j) {
                    tree.erase(j);
                }
            }, i * n_numbers);
        }

        for (int i = 0; i < n_threads; ++i) {
            vt[i].join();
        }

        REQUIRE(tree.empty(), "case 1");
    }

    void erase_from_different_sides() {
        test_case = "erase_from_different_sides";

        consistent_tree<int> tree;

        int n_numbers = 1e5;
        for (int i = 0; i < n_numbers; ++i) {
            tree.insert(i);
        }

        std::vector<std::thread> vt(n_threads);

        for (int i = 0; i < vt.size(); ++i) {
            vt[i] = std::thread([&](int side) -> void {
                if (side) {
                    for (int j = 0; j < n_numbers; j++) {
                        tree.erase(j);
                    }
                } else {
                    for (int j = n_numbers - 1; j >= 0; j--) {
                        tree.erase(j);
                    }
                }
            }, i % 2);
        }

        for (int i = 0; i < n_threads; ++i) {
            vt[i].join();
        }

        REQUIRE(tree.empty(), "case 1");
    }

    void find() {
        test_case = "erase_from_different_sides";

        consistent_tree<int> tree;

        int n_numbers = 1e5;
        for (int i = 0; i < n_numbers; ++i) {
            tree.insert(i);
        }

        std::vector<std::thread> vt(n_threads);

        for (int i = 0; i < vt.size(); ++i) {
            vt[i] = std::thread([&]() -> void {
                for (int j = 0; j < n_numbers; j++) {
                    REQUIRE(tree.find(j) != tree.end());
                    REQUIRE(tree.find(-j - 1) == tree.end());
                }
            });
        }

        for (int i = 0; i < n_threads; ++i) {
            vt[i].join();
        }
    }

    void run() {
        std::cout << "--coarse_grained_test.h--\n";
        std::cout << n_threads << " threads\n";

        insert_one_number();
        insert_same_numbers();
        insert_different_numbers();

        erase_same_numbers();
        erase_different_numbers();
        erase_from_different_sides();

        find();

        std::cout << test_counter - fail_counter << " TEST PASSED\n";
        std::cout << fail_counter << " TEST FAILED\n";
        std::cout << "-------------------------\n\n";
    }
};
