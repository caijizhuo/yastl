#include <iostream>
#include "vector.h"
#include "iterator.h"
int main()
{
    std::cout.sync_with_stdio(false);
    yastl::vector<int> v = {1, 2, 3};
    for (auto i : v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    v.reserve(1000);



    std::cout << "end!" << std::endl;
}
