#include <iostream>
#include "vector.h"
#include "iterator.h"
#include "util.h"

yastl::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9};

template <typename T>
void printv(yastl::vector<T> v) {
    for (auto i : v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

void func() {
    yastl::vector<int> v2 = {3, 2, 1};
    v.swap(v2);
    printv(v);
    printv(v2);
}

int main()
{
    std::cout.sync_with_stdio(false);
    printv(v);
    func();
    printv(v);

    std::cout << "end!" << std::endl;
}
