#include <iostream>
#include "stack.h"
#include "iterator.h"
#include "util.h"
int main()
{
    yastl::stack<int> s;
    s.push(10);
    std::cout << s.top() << std::endl;
    s.push(20);
    std::cout << s.top() << std::endl;
    s.pop();
    std::cout << s.top() << std::endl;

    std::cout << "end!" << std::endl;
}
