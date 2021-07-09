#include "tests/tree_test.h"
#include "tests/coarse_grained_test.h"

int main() {
    tree_test().run();
    coarse_grained_test(4).run();
    return 0;
}
