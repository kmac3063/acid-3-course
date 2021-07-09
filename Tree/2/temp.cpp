#include <random>
#include <ctime>
#include <vector>
#include <iostream>
#include <set>

using namespace std;

struct node {
    int val = 0;
    node() = default;
    node(int v) : val(v) {}
};

void f(node** from, node* to) {
    *from = to;
}

struct iterator1 {
    node* current_node;

    iterator1(node* n) {
        current_node = n;
    }

    bool operator==(const iterator1& rhs) const {
        return &(*current_node) == &(*rhs.current_node);
    }
};

int main() {
    node* a = new node(1);
    node* b = new node(1);

    iterator1 it1(a);
    iterator1 it2(b);

    cout << ((int)(it1 == it2));

    return 0;
}
