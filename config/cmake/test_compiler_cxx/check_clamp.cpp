#include <algorithm>

int main(int argc, char */*argv*/[])
{
    auto res=std::clamp(argc, 0, 3);
    return res;
}

