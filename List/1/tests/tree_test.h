#include <string>

#include "fail_printer.h"

class tree_test {
private:
    void print_fail(const std::string& test_name, const std::string& reason = "") {
        fail_printer::print("tree_test.h", test_name, reason);
    }

    void insert() {

    }

    void find() {}
    void erase() {}
    void to_vector() {}

    void begin() {}
    void end() {}
    void iter_pp() {}
    void pp_iter() {}

public:
    void run() {
        insert();
        find();
        erase();
        to_vector();

        begin();
        end();
        iter_pp();
        pp_iter();
    }
};