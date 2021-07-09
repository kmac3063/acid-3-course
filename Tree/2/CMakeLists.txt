cmake_minimum_required(VERSION 3.17)
project(1)

set(CMAKE_CXX_STANDARD 17)


add_executable(1
#        main_all_in_one_file.cpp

        main.cpp
        tests/fail_printer.h
        tests/tree_test.h
        tests/coarse_grained_test.h
        consistent_tree.h
        utils.h
        )
