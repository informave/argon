#include <argon/dtsengine>

#include <iostream>

int main(void)
{
    std::locale::global(std::locale(""));

    std::ios_base::sync_with_stdio(true);
    std::cout.setf(std::ios::unitbuf);
    std::wcout.setf(std::ios::unitbuf);


    using namespace informave::db;
    using namespace informave::argon;
    

    DTSEngine engine;

    return 0;
}


