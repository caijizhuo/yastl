#include <iostream>
#include "vector.h"
#include "iterator.h"
int main()
{
    std::cout.sync_with_stdio(false);
    yastl::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (auto i : v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    auto first = v.begin() + 2;
    std::cout << "first!" << first << std::endl;
    auto last = v.begin() + 5;
    std::cout << "first2!" << v.erase(first, last) << std::endl;
    for (auto i : v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    std::cout << "end!" << std::endl;
}
