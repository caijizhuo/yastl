#include <iostream>
#include "rb_tree.h"
#include "set.h"
#include "map.h"
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

    yastl::set<int, std::less<int>> s1;
    s1.insert(1);
    s1.insert(2);
    for (auto i : s1) {
        std::cout << i << std::endl;
    }

    yastl::map<int, std::string> mp;
    mp[1] = "1111";
    mp[2] = "2222";
    for (auto p : mp) {
        std::cout << p.first << ":" << p.second << std::endl;
    }


    std::cout << "end!" << std::endl;
}
