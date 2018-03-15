#include <shared_mutex>

int main()
{
    std::shared_mutex mtx;
    mtx.lock_shared();
    mtx.unlock_shared();
}

