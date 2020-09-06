#include "Dictionary.h"

#include <iostream>

int main()
{
    std::vector<int> v = {1, 2, 3, 3, 4, 5, 12, 9, 0, 0, 0, 1, 2, 2, 3, 4, 5, 7, 12 };
    for (const auto& val: v)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    Dictionary<int, 3> d;
    d.addDataToDictionary<decltype(v)>(v.begin(), v.end(), 3);

    d.printNodes();

    std::vector<const int*> vv;
    for (int i = 0; i < 5; ++i)
    {
        const auto* genVal =  d.getToken<decltype(vv)>(vv.begin(), vv.end(), 2);
        if (not genVal)
            break;
        vv.push_back(genVal);
    }
    for (const auto& val: vv)
    {
        std::cout << *val << " ";
    }
    std::cout << std::endl;

    return 0;
}
