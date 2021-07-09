#include <algorithm>
#include <ctime>

#include "func_tests.h"
#include "thread_with_lock_list_tests.h"

using namespace std;

int main() {
    srand(time(nullptr));

    func_tests::start();
    threads_with_lock_list_tests::start();

    return 0;
}
