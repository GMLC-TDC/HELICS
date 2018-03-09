
template<class T>
constexpr T pi = T(3.1415926535897932385L);  // variable template

int main()
{
    return pi<int>;
}

