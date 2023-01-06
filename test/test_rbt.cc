#include <iostream>
#include "rb_tree.h"
#include "iterator.h"
#include "util.h"
int main()
{
    yastl::rb_tree<int, std::less<int>()> rb;

    std::cout << "end!" << std::endl;
}
