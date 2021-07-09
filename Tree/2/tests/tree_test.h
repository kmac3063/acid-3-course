#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>

#include "../consistent_tree.h"
#include "fail_printer.h"
#include "../utils.h"

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

        for (int i = (int) N - 1; i >= 0; i--) {
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

        for (int i = (int) v1.size() - 1; i >= 0; i--) {
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
        for (i = (int) v_it.size() - 1; i >= 0; i--) {
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