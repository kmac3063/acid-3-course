#include "consistent_linked_list.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <time.h>

#include "func_tests.h"

using namespace std;

int main() {
    srand(time(NULL));

    tree_test().run();
    coarse_grained_test().run()
    medium_grained_test().run()
    fine_grained_test().run()
    spinlock_based_test().run()
    consistent_acces_test().run()
    durable_writes_test().run()

    return 0;
}
