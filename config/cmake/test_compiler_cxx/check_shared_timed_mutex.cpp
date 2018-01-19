#include <shared_mutex>

int main()
{
    std::shared_timed_mutex mtx;
    mtx.lock_shared();
    mtx.unlock_shared();
}