#include <algorithm>

int main(int argc, char */*argv*/[])
{
    auto res = std::hypot(static_cast<double>(argc), 2.3, 1.7);
    return static_cast<int>(res);
}

