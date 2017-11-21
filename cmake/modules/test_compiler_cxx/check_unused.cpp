
#include <cassert>
int main(int argc, char *argv[])
{
    [[maybe_unused]] bool b = (argc>3);
    assert(b); // in release mode, assert is compiled out, and b is unused
               // no warning because it is declared [[maybe_unused]]
}