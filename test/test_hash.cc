#include <iostream>
#include "hashtable.h"
#include "unordered_map.h"
#include "iterator.h"
#include "functional.h"
#include "util.h"
int main()
{
    yastl::hashtable<int, std::hash<int>, yastl::equal_to<int>> ht1(10, std::hash<int>(), yastl::equal_to<int>());
    yastl::hashtable<int, std::hash<int>, yastl::equal_to<int>> ht2(10, std::hash<int>(), yastl::equal_to<int>());
    ht1.insert_multi(1);
    ht1.insert_multi(2);
    ht1.insert_multi(3);
    ht1.insert_multi(4);

    ht2.insert_multi(1);
    ht2.insert_multi(2);
    ht2.insert_multi(3);
    ht2.insert_multi(5);
    yastl::unordered_map<int, int> mp;
    mp[3] = 4;
    std::cout << "end!" << std::endl;
}
