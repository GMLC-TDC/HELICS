#define CX 1
int main()
{
    if constexpr (CX == 1)
        return 1;
    else
        return 0;
}