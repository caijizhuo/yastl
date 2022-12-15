#include <iostream>
#include "list.h"
#include "iterator.h"
#include "util.h"
template <typename T>
void printl(yastl::list<T> l) {
    for (auto i :l) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}
int main()
{
    std::cout.sync_with_stdio(false);
    yastl::list<int> l;
    l.assign(10, 1);
    printl(l);


    std::cout << "end!" << std::endl;
}
