#include <iostream>
#include "rb_tree.h"
#include "iterator.h"
#include "util.h"
int main()
{
    yastl::rb_tree<int, std::less<int>> rb;
    rb.insert_multi(10);
    rb.insert_multi(1);
    rb.insert_multi(2);
    rb.insert_multi(3);
    rb.insert_multi(4);

    std::cout << "end!" << std::endl;
}
