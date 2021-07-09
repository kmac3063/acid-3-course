#include <string>
#include <iostream>
#include <cstdio>


class fail_printer {
private:
    using csr = const std::string&;
public:
    static void print(csr file_name, csr test_name, csr reason) {
        printf("Fail. File: %10s. Test: %s. ", file_name.c_str(), test_name.c_str());

        if (!reason.empty()) {
            printf("Reason: %s.", reason.c_str());
        }
        printf("\n");
    }
};