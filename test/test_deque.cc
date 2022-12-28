#include <iostream>
#include "deque.h"
#include "iterator.h"
#include "util.h"
int main()
{
    yastl::deque<int> dq(3999, 9);
    std::cout << dq[2000];


    std::cout << "end!" << std::endl;
}
