#include <iostream>
#include "deque.h"
#include "iterator.h"
#include "util.h"
int main()
{
    yastl::deque<int> dq(10, 9);
    std::cout << dq[0] << " " << dq[9] << std::endl;

    dq.pop_front();
    dq.push_back(99);
    std::cout << dq[0] << " " << dq[9] << std::endl;

    std::cout << "end!" << std::endl;
}
