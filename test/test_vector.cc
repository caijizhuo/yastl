#include <iostream>
#include "vector.h"
#include "iterator.h"
#include "util.h"
template <typename T>
void printv(yastl::vector<T> v) {
    for (auto i : v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}
int main()
{
    std::cout.sync_with_stdio(false);
    yastl::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    yastl::vector<int> v2 = {3, 2, 1};
    printv(v);
    printv(v2);
    v.swap(v2);

    printv(v);
    printv(v2);

    std::cout << "end!" << std::endl;
}
