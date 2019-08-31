/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "gtest/gtest.h"

#include "helics/core/ipc/IpcBlockingPriorityQueueImpl.hpp"
#include <future>
#include <memory>
#include <thread>

using namespace std::literals::chrono_literals;

TEST (IpcQueueImpl_tests, creation_test)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[10192]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 10192);

    std::vector<unsigned char> data (500, 'a');
    EXPECT_TRUE (queue.try_push (data.data (), 500));

    data.assign (500, 'b');
    int res = queue.pop (data.data (), 500);
    EXPECT_EQ (res, 500);
    EXPECT_EQ (data[234], 'a');
    EXPECT_EQ (data[499], 'a');
}

TEST (IpcQueueImpl_tests, push_pop_tests)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[10192]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 10192);

    std::vector<unsigned char> data (500, 'a');
    EXPECT_TRUE (queue.try_push (data.data (), 400));  // this would go into the pull
    queue.push (data.data (), 401);  // this goes into push
    queue.push (data.data (), 402);  // this goes into push
    queue.push (data.data (), 403);  // this goes into push

    int sz = queue.pop (data.data (), 500);  // pop from pull
    EXPECT_EQ (sz, 400);
    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    EXPECT_EQ (sz, 401);  // pop from pull
    queue.push (data.data (), 404);  // this goes into push
    queue.push (data.data (), 405);  // this goes into push
    queue.push (data.data (), 406);  // this goes into push

    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    EXPECT_EQ (sz, 402);  // pop from pull

    sz = queue.pop (data.data (), 500);
    EXPECT_EQ (sz, 403);

    sz = queue.pop (data.data (), 500);
    EXPECT_EQ (sz, 404);
    sz = queue.pop (data.data (), 500);
    EXPECT_EQ (sz, 405);
    sz = queue.pop (data.data (), 500);
    EXPECT_EQ (sz, 406);
}

TEST (IpcQueueImpl_tests, push_pop_priority_tests)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[10192]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 10192);

    std::vector<unsigned char> data (500, 'a');
    EXPECT_TRUE (queue.try_push (data.data (), 400));  // this would go into the pull
    queue.push (data.data (), 401);  // this goes into push
    queue.push (data.data (), 402);  // this goes into push
    queue.push (data.data (), 403);  // this goes into push

    int sz = queue.pop (data.data (), 500);  // pop from pull
    EXPECT_EQ (sz, 400);
    queue.pushPriority (data.data (), 417);
    sz = queue.pop (data.data (), 500);  // pull from priority
    EXPECT_EQ (sz, 417);  // pop from pull
    sz = queue.pop (data.data (), 500);  // pull from priority
    EXPECT_EQ (sz, 401);  // pop from pull
    queue.push (data.data (), 404);  // this goes into push
    queue.pushPriority (data.data (), 420);  // this goes into priority
    queue.pushPriority (data.data (), 421);  // this goes into priority
    queue.push (data.data (), 405);  // this goes into push
    queue.push (data.data (), 406);  // this goes into push

    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    EXPECT_EQ (sz, 420);  // pop from priority
    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    EXPECT_EQ (sz, 421);  // pop from priority

    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    EXPECT_EQ (sz, 402);  // pop from pull

    sz = queue.pop (data.data (), 500);
    EXPECT_EQ (sz, 403);

    sz = queue.pop (data.data (), 500);
    EXPECT_EQ (sz, 404);
    sz = queue.pop (data.data (), 500);
    EXPECT_EQ (sz, 405);
    EXPECT_TRUE (!queue.empty ());
    sz = queue.try_pop (data.data (), 500);
    EXPECT_EQ (sz, 406);

    EXPECT_TRUE (queue.empty ());
}

TEST (IpcQueueImpl_tests, push_full)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');
    EXPECT_TRUE (queue.try_push (data.data (), 420));  // this would go into the pull
    EXPECT_TRUE (queue.try_push (data.data (), 420));  // this would go into the push
    EXPECT_TRUE (queue.try_push (data.data (), 420));  // this would go into the push
    EXPECT_TRUE (queue.try_push (data.data (), 420));  // this would go into the push

    EXPECT_TRUE (!queue.try_push (data.data (), 420));  // this should fail as it is full
    EXPECT_EQ (queue.push (std::chrono::milliseconds (50), data.data (), 420),
               0);  // this should return 0 as timeout
}

TEST (IpcQueueImpl_tests, push_priority_full)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');
    EXPECT_TRUE (queue.try_pushPriority (data.data (), 390));  // this would go into the pull
    EXPECT_TRUE (queue.try_pushPriority (data.data (), 390));  // this would go into the push

    EXPECT_TRUE (!queue.try_pushPriority (data.data (), 390));  // this should fail as it is full
    EXPECT_EQ (queue.pushPriority (std::chrono::milliseconds (50), data.data (), 420),
               0);  // this should return 0 as timeout
}

TEST (IpcQueueImpl_tests, pop_wait)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');
    EXPECT_EQ (queue.try_pop (data.data (), 390), 0);  // this would go into the pull

    EXPECT_EQ (queue.pop (std::chrono::milliseconds (100), data.data (), 420),
               0);  // this should return 0 as timeout
}

TEST (IpcQueueImpl_tests, pop_wait2)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');

    auto def = std::async (std::launch::async, [&]() {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        queue.push (data.data (), 300);
    });

    int ret = queue.pop (data.data (), 500);
    EXPECT_EQ (ret, 300);
    def.wait ();
}

TEST (IpcQueueImpl_tests, pop_wait_priority)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');

    auto def = std::async (std::launch::async, [&]() {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        queue.pushPriority (data.data (), 300);
    });

    int ret = queue.pop (data.data (), 500);
    EXPECT_EQ (ret, 300);
    def.wait ();
}

TEST (IpcQueueImpl_tests, push_wait)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');
    std::vector<unsigned char> data2 (500, 'a');
    EXPECT_TRUE (queue.try_push (data.data (), 420));  // this would go into the pull
    EXPECT_TRUE (queue.try_push (data.data (), 420));  // this would go into the push
    EXPECT_TRUE (queue.try_push (data.data (), 420));  // this would go into the push
    EXPECT_TRUE (queue.try_push (data.data (), 420));  // this would go into the push

    EXPECT_TRUE (!queue.try_push (data.data (), 420));  // this should fail as it is full
    auto def = std::async (std::launch::async, [&]() {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        queue.pop (data.data (), 500);
    });
    queue.push (data.data (), 420);
    def.wait ();
}

TEST (IpcQueueImpl_tests, priority_push_wait)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');
    std::vector<unsigned char> data2 (500, 'a');

    EXPECT_TRUE (queue.try_pushPriority (data.data (), 390));  // this would go into the pull
    EXPECT_TRUE (queue.try_pushPriority (data.data (), 390));  // this would go into the push

    EXPECT_TRUE (!queue.try_pushPriority (data.data (), 390));  // this should fail as it is full

    auto def = std::async (std::launch::async, [&]() {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        queue.pop (data.data (), 500);
    });
    queue.pushPriority (data.data (), 390);
    def.wait ();
}
/** test with single consumer/single producer*/

TEST (IpcQueueImpl_tests, multithreaded_tests)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[1048576]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 1048576);

    for (int64_t ii = 0; ii < 10'000; ++ii)
    {
        queue.push (reinterpret_cast<unsigned char *> (&ii), 8);
    }

    auto prod1 = [&]() {
        int64_t bdata;
        for (int64_t jj = 10'000; jj < 1'010'000; ++jj)
        {
            bdata = jj;
            queue.push (reinterpret_cast<unsigned char *> (&bdata), 8);
        }
    };

    auto cons = [&]() {
        int64_t data;
        auto res = queue.try_pop (reinterpret_cast<unsigned char *> (&data), 8);
        int64_t cnt = 0;
        while ((res))
        {
            ++cnt;
            res = queue.try_pop (reinterpret_cast<unsigned char *> (&data), 8);
            if (res == 0)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                res = queue.try_pop (reinterpret_cast<unsigned char *> (&data), 8);
            }
        }
        return cnt;
    };

    auto ret = std::async (std::launch::async, prod1);

    auto res = std::async (std::launch::async, cons);

    ret.wait ();
    auto V = res.get ();
    EXPECT_EQ (V, 1'010'000);
}

/** test with multiple consumer/single producer*/
TEST (IpcQueueImpl_tests, multithreaded_tests2)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[1048576]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 1048576);
    for (int64_t ii = 0; ii < 10'000; ++ii)
    {
        queue.push (reinterpret_cast<unsigned char *> (&ii), 8);
    }
    auto prod1 = [&]() {
        int64_t bdata;
        for (int64_t jj = 10'000; jj < 1'010'000; ++jj)
        {
            bdata = jj;
            queue.push (reinterpret_cast<unsigned char *> (&bdata), 8);
        }
    };

    auto cons = [&]() {
        int64_t data;
        auto res = queue.try_pop (reinterpret_cast<unsigned char *> (&data), 8);
        int64_t cnt = 0;
        while ((res))
        {
            ++cnt;
            res = queue.try_pop (reinterpret_cast<unsigned char *> (&data), 8);
            if (res == 0)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                res = queue.try_pop (reinterpret_cast<unsigned char *> (&data), 8);
            }
        }
        return cnt;
    };

    auto ret = std::async (std::launch::async, prod1);

    auto res1 = std::async (std::launch::async, cons);
    auto res2 = std::async (std::launch::async, cons);
    auto res3 = std::async (std::launch::async, cons);
    ret.wait ();
    auto V1 = res1.get ();
    auto V2 = res2.get ();
    auto V3 = res3.get ();

    EXPECT_EQ (V1 + V2 + V3, 1'010'000);
}

/** test with multiple producer/multiple consumer*/
TEST (IpcQueueImpl_tests, multithreaded_tests3)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[1048576]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 1048576);
    for (int64_t ii = 0; ii < 10'000; ++ii)
    {
        queue.push (reinterpret_cast<unsigned char *> (&ii), 8);
    }
    auto prod1 = [&]() {
        int64_t bdata;
        for (int64_t jj = 10'000; jj < 1'010'000; ++jj)
        {
            bdata = jj;
            queue.push (reinterpret_cast<unsigned char *> (&bdata), 8);
        }
    };

    auto cons = [&]() {
        int64_t data;
        auto res = queue.try_pop (reinterpret_cast<unsigned char *> (&data), 8);
        int64_t cnt = 0;
        while ((res))
        {
            ++cnt;
            res = queue.try_pop (reinterpret_cast<unsigned char *> (&data), 8);
            if (res == 0)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                res = queue.try_pop (reinterpret_cast<unsigned char *> (&data), 8);
            }
        }
        return cnt;
    };

    auto ret1 = std::async (std::launch::async, prod1);
    auto ret2 = std::async (std::launch::async, prod1);
    auto ret3 = std::async (std::launch::async, prod1);

    auto res1 = std::async (std::launch::async, cons);
    auto res2 = std::async (std::launch::async, cons);
    auto res3 = std::async (std::launch::async, cons);
    ret1.wait ();
    ret2.wait ();
    ret3.wait ();
    auto V1 = res1.get ();
    auto V2 = res2.get ();
    auto V3 = res3.get ();

    EXPECT_EQ (V1 + V2 + V3, 3'010'000);
}

/** test with multiple producer/multiple consumer*/
TEST (IpcQueueImpl_tests, multithreaded_tests3_pop)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[1048576]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 1048576);

    auto prod1 = [&]() {
        int64_t bdata;
        for (int64_t jj = 0; jj < 1'000'000; ++jj)
        {
            bdata = jj;
            queue.push (reinterpret_cast<unsigned char *> (&bdata), 8);
        }
        bdata = (-1);
        queue.push (reinterpret_cast<unsigned char *> (&bdata), 8);
    };

    auto cons = [&]() {
        int64_t data = 0;
        int64_t cnt = 0;
        while (data >= 0)
        {
            ++cnt;
            queue.pop (reinterpret_cast<unsigned char *> (&data), 8);
        }
        return cnt;
    };

    auto ret1 = std::async (std::launch::async, prod1);
    auto ret2 = std::async (std::launch::async, prod1);
    auto ret3 = std::async (std::launch::async, prod1);

    auto res1 = std::async (std::launch::async, cons);
    auto res2 = std::async (std::launch::async, cons);
    auto res3 = std::async (std::launch::async, cons);
    ret1.wait ();
    ret2.wait ();
    ret3.wait ();
    auto V1 = res1.get ();
    auto V2 = res2.get ();
    auto V3 = res3.get ();

    EXPECT_EQ (V1 + V2 + V3, 3'000'003);
}

TEST (IpcQueueImpl_tests, push_full_mem)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    std::unique_ptr<unsigned char[]> memblock2 (new unsigned char[4096]);

    auto *queue = new (memblock2.get ()) helics::ipc::detail::IpcBlockingPriorityQueueImpl (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');
    EXPECT_TRUE (queue->try_push (data.data (), 420));  // this would go into the pull
    EXPECT_TRUE (queue->try_push (data.data (), 420));  // this would go into the push
    EXPECT_TRUE (queue->try_push (data.data (), 420));  // this would go into the push
    EXPECT_TRUE (queue->try_push (data.data (), 420));  // this would go into the push

    EXPECT_TRUE (!queue->try_push (data.data (), 420));  // this should fail as it is full
    EXPECT_EQ (queue->push (std::chrono::milliseconds (50), data.data (), 420),
               0);  // this should return 0 as timeout
}

/** test with multiple producer/multiple consumer*/
TEST (IpcQueueImpl_tests, multithreaded_tests3_pop_mem)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[1048576]);

    std::unique_ptr<unsigned char[]> memblock2 (new unsigned char[4096]);

    auto *qn = new (memblock2.get ()) helics::ipc::detail::IpcBlockingPriorityQueueImpl (memblock.get (), 1048576);

    auto prod1 = [&](void *data) {
        auto *queue = reinterpret_cast<helics::ipc::detail::IpcBlockingPriorityQueueImpl *> (data);
        int64_t bdata;
        for (int64_t jj = 0; jj < 1'000'000; ++jj)
        {
            bdata = jj;
            queue->push (reinterpret_cast<unsigned char *> (&bdata), 8);
        }
        bdata = (-1);
        queue->push (reinterpret_cast<unsigned char *> (&bdata), 8);
    };

    auto cons = [&](void *mem) {
        auto *queue = reinterpret_cast<helics::ipc::detail::IpcBlockingPriorityQueueImpl *> (mem);
        int64_t data = 0;
        int64_t cnt = 0;
        while (data >= 0)
        {
            ++cnt;
            queue->pop (reinterpret_cast<unsigned char *> (&data), 8);
        }
        return cnt;
    };

    auto ret1 = std::async (std::launch::async, prod1, memblock2.get ());
    auto ret2 = std::async (std::launch::async, prod1, memblock2.get ());
    auto ret3 = std::async (std::launch::async, prod1, memblock2.get ());

    auto res1 = std::async (std::launch::async, cons, memblock2.get ());
    auto res2 = std::async (std::launch::async, cons, memblock2.get ());
    auto res3 = std::async (std::launch::async, cons, memblock2.get ());
    ret1.wait ();
    ret2.wait ();
    ret3.wait ();
    auto V1 = res1.get ();
    auto V2 = res2.get ();
    auto V3 = res3.get ();

    EXPECT_EQ (V1 + V2 + V3, 3'000'003);
}
