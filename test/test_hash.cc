#include <iostream>
#include "hashtable.h"
#include "iterator.h"
#include "functional.h"
#include "util.h"
int main()
{
    yastl::hashtable<int, std::hash<int>, yastl::equal_to<int>> ht(10, std::hash<int>(), yastl::equal_to<int>());


    std::cout << "end!" << std::endl;
}
