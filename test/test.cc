#include <iostream>
#include "vector.h"
#include "iterator.h"
int main()
{
    std::cout.sync_with_stdio(false);
    yastl::vector<int> v = {1, 2, 3};
    // for (auto i : v) {
    //     std::cout << i << " ";
    // }
    std::cout << v.rbegin().base() << " " << (v.rbegin() + 1).base() << std::endl;
    std::cout << (v.rbegin() < (v.rbegin() + 1)) << std::endl;
    std::cout << (v.rbegin() <= (v.rbegin() + 1)) << std::endl;
    std::cout << (v.rbegin() > (v.rbegin() + 1)) << std::endl;
    std::cout << (v.rbegin() >= (v.rbegin() + 1)) << std::endl;
}
