/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>

#include "helics/core/ipc/IpcBlockingPriorityQueueImpl.hpp"
#include <future>
#include <memory>
#include <thread>

namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE (IpcQueue_tests, *utf::label ("ci"))

/*
BOOST_AUTO_TEST_CASE (creation_test)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[10192]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 10192);

    std::vector<unsigned char> data (500, 'a');
    BOOST_CHECK (queue.try_push (data.data (), 500));

    data.assign (500, 'b');
    int res = queue.pop (data.data (), 500);
    BOOST_CHECK_EQUAL (res, 500);
    BOOST_CHECK_EQUAL (data[234], 'a');
    BOOST_CHECK_EQUAL (data[499], 'a');
}

BOOST_AUTO_TEST_CASE (push_pop_tests)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[10192]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 10192);

    std::vector<unsigned char> data (500, 'a');
    BOOST_CHECK (queue.try_push (data.data (), 400));  // this would go into the pull
    queue.push (data.data (), 401);  // this goes into push
    queue.push (data.data (), 402);  // this goes into push
    queue.push (data.data (), 403);  // this goes into push

    int sz = queue.pop (data.data (), 500);  // pop from pull
    BOOST_CHECK_EQUAL (sz, 400);
    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    BOOST_CHECK_EQUAL (sz, 401);  // pop from pull
    queue.push (data.data (), 404);  // this goes into push
    queue.push (data.data (), 405);  // this goes into push
    queue.push (data.data (), 406);  // this goes into push

    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    BOOST_CHECK_EQUAL (sz, 402);  // pop from pull

    sz = queue.pop (data.data (), 500);
    BOOST_CHECK_EQUAL (sz, 403);

    sz = queue.pop (data.data (), 500);
    BOOST_CHECK_EQUAL (sz, 404);
    sz = queue.pop (data.data (), 500);
    BOOST_CHECK_EQUAL (sz, 405);
    sz = queue.pop (data.data (), 500);
    BOOST_CHECK_EQUAL (sz, 406);
}

BOOST_AUTO_TEST_CASE (push_pop_priority_tests)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[10192]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 10192);

    std::vector<unsigned char> data (500, 'a');
    BOOST_CHECK (queue.try_push (data.data (), 400));  // this would go into the pull
    queue.push (data.data (), 401);  // this goes into push
    queue.push (data.data (), 402);  // this goes into push
    queue.push (data.data (), 403);  // this goes into push

    int sz = queue.pop (data.data (), 500);  // pop from pull
    BOOST_CHECK_EQUAL (sz, 400);
    queue.pushPriority (data.data (), 417);
    sz = queue.pop (data.data (), 500);  // pull from priority
    BOOST_CHECK_EQUAL (sz, 417);  // pop from pull
    sz = queue.pop (data.data (), 500);  // pull from priority
    BOOST_CHECK_EQUAL (sz, 401);  // pop from pull
    queue.push (data.data (), 404);  // this goes into push
    queue.pushPriority (data.data (), 420);  // this goes into priority
    queue.pushPriority (data.data (), 421);  // this goes into priority
    queue.push (data.data (), 405);  // this goes into push
    queue.push (data.data (), 406);  // this goes into push

    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    BOOST_CHECK_EQUAL (sz, 420);  // pop from priority
    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    BOOST_CHECK_EQUAL (sz, 421);  // pop from priority

    sz = queue.pop (data.data (), 500);  // pull empty, so rotate
    BOOST_CHECK_EQUAL (sz, 402);  // pop from pull

    sz = queue.pop (data.data (), 500);
    BOOST_CHECK_EQUAL (sz, 403);

    sz = queue.pop (data.data (), 500);
    BOOST_CHECK_EQUAL (sz, 404);
    sz = queue.pop (data.data (), 500);
    BOOST_CHECK_EQUAL (sz, 405);
    BOOST_CHECK (!queue.empty ());
    sz = queue.try_pop (data.data (), 500);
    BOOST_CHECK_EQUAL (sz, 406);

    BOOST_CHECK (queue.empty ());
}

BOOST_AUTO_TEST_CASE (push_full)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');
    BOOST_CHECK (queue.try_push (data.data (), 420));  // this would go into the pull
    BOOST_CHECK (queue.try_push (data.data (), 420));  // this would go into the push
    BOOST_CHECK (queue.try_push (data.data (), 420));  // this would go into the push
    BOOST_CHECK (queue.try_push (data.data (), 420));  // this would go into the push

    BOOST_CHECK (!queue.try_push (data.data (), 420));  // this should fail as it is full
    BOOST_CHECK_EQUAL (queue.push (std::chrono::milliseconds (50), data.data (), 420),
                       0);  // this should return 0 as timeout
}

BOOST_AUTO_TEST_CASE (push_priority_full)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');
    BOOST_CHECK (queue.try_pushPriority (data.data (), 390));  // this would go into the pull
    BOOST_CHECK (queue.try_pushPriority (data.data (), 390));  // this would go into the push

    BOOST_CHECK (!queue.try_pushPriority (data.data (), 390));  // this should fail as it is full
    BOOST_CHECK_EQUAL (queue.pushPriority (std::chrono::milliseconds (50), data.data (), 420),
                       0);  // this should return 0 as timeout
}

BOOST_AUTO_TEST_CASE (pop_wait)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');
    BOOST_CHECK_EQUAL (queue.try_pop (data.data (), 390), 0);  // this would go into the pull

    BOOST_CHECK_EQUAL (queue.pop (std::chrono::milliseconds (100), data.data (), 420),
                       0);  // this should return 0 as timeout
}

BOOST_AUTO_TEST_CASE (pop_wait2)
{
    std::unique_ptr<unsigned char[]> memblock (new unsigned char[4096]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue (memblock.get (), 4096);
    std::vector<unsigned char> data (500, 'a');

    auto def = std::async (std::launch::async, [&]() {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        queue.push (data.data (), 300);
    });

    int ret = queue.pop (data.data (), 500);
    BOOST_CHECK_EQUAL (ret, 300);
	def.wait();
}

BOOST_AUTO_TEST_CASE(pop_wait_priority)
{
	std::unique_ptr<unsigned char[]> memblock(new unsigned char[4096]);

	helics::ipc::detail::IpcBlockingPriorityQueueImpl queue(memblock.get(), 4096);
	std::vector<unsigned char> data(500, 'a');

	auto def = std::async(std::launch::async, [&]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		queue.pushPriority(data.data(), 300);
	});

	int ret = queue.pop(data.data(), 500);
	BOOST_CHECK_EQUAL(ret, 300);
	def.wait();
}


BOOST_AUTO_TEST_CASE(push_wait)
{
	std::unique_ptr<unsigned char[]> memblock(new unsigned char[4096]);

	helics::ipc::detail::IpcBlockingPriorityQueueImpl queue(memblock.get(), 4096);
	std::vector<unsigned char> data(500, 'a');
	std::vector<unsigned char> data2(500, 'a');
	BOOST_CHECK(queue.try_push(data.data(), 420));  // this would go into the pull
	BOOST_CHECK(queue.try_push(data.data(), 420));  // this would go into the push
	BOOST_CHECK(queue.try_push(data.data(), 420));  // this would go into the push
	BOOST_CHECK(queue.try_push(data.data(), 420));  // this would go into the push

	BOOST_CHECK(!queue.try_push(data.data(), 420));  // this should fail as it is full
	auto def = std::async(std::launch::async, [&]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		queue.pop(data.data(), 500);
	});
	queue.push(data.data(), 420);
	def.wait();
}

BOOST_AUTO_TEST_CASE(priority_push_wait)
{
	std::unique_ptr<unsigned char[]> memblock(new unsigned char[4096]);

	helics::ipc::detail::IpcBlockingPriorityQueueImpl queue(memblock.get(), 4096);
	std::vector<unsigned char> data(500, 'a');
	std::vector<unsigned char> data2(500, 'a');

	BOOST_CHECK(queue.try_pushPriority(data.data(), 390));  // this would go into the pull
	BOOST_CHECK(queue.try_pushPriority(data.data(), 390));  // this would go into the push

	BOOST_CHECK(!queue.try_pushPriority(data.data(), 390));  // this should fail as it is full

	auto def = std::async(std::launch::async, [&]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		queue.pop(data.data(), 500);
	});
	queue.pushPriority(data.data(), 390);
	def.wait();
}
*/
/** test with single consumer/single producer*/

/*
BOOST_AUTO_TEST_CASE(multithreaded_tests)
{
    std::unique_ptr<unsigned char[]> memblock(new unsigned char[1048576]);

    helics::ipc::detail::IpcBlockingPriorityQueueImpl queue(memblock.get(), 1048576);


    for (int64_t ii = 0; ii < 10'000; ++ii)
    {
        queue.push(reinterpret_cast<unsigned char *>(&ii),8);
    }

    auto prod1 = [&]() {
        int64_t bdata;
        for (int64_t jj = 10'000; jj < 1'010'000; ++jj)
        {
            bdata = jj;
            queue.push(reinterpret_cast<unsigned char *>(&bdata), 8);
        }
    };

    auto cons = [&]() {
        int64_t data;
        auto res = queue.try_pop(reinterpret_cast<unsigned char *>(&data),8);
        int64_t cnt = 0;
        while ((res))
        {
            ++cnt;
            res = queue.try_pop(reinterpret_cast<unsigned char *>(&data), 8);
            if (res==0)
            {  // make an additional sleep period so the producer can catch up
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                res = queue.try_pop(reinterpret_cast<unsigned char *>(&data), 8);
            }
        }
        return cnt;
    };

    auto ret = std::async(std::launch::async, prod1);

    auto res = std::async(std::launch::async, cons);

    ret.wait();
    auto V = res.get();
    BOOST_CHECK_EQUAL(V, 1'010'000);
}
*/
/** test with multiple consumer/single producer*/
/*
BOOST_AUTO_TEST_CASE(multithreaded_tests2)
{
	std::unique_ptr<unsigned char[]> memblock(new unsigned char[1048576]);

	helics::ipc::detail::IpcBlockingPriorityQueueImpl queue(memblock.get(), 1048576);
	for (int64_t ii = 0; ii < 10'000; ++ii)
	{
		queue.push(reinterpret_cast<unsigned char *>(&ii), 8);
	}
	auto prod1 = [&]() {
		int64_t bdata;
		for (int64_t jj = 10'000; jj < 1'010'000; ++jj)
		{
			bdata = jj;
			queue.push(reinterpret_cast<unsigned char *>(&bdata), 8);
		}
	};

	auto cons = [&]() {
		int64_t data;
		auto res = queue.try_pop(reinterpret_cast<unsigned char *>(&data), 8);
		int64_t cnt = 0;
		while ((res))
		{
			++cnt;
			res = queue.try_pop(reinterpret_cast<unsigned char *>(&data), 8);
			if (res == 0)
			{  // make an additional sleep period so the producer can catch up
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				res = queue.try_pop(reinterpret_cast<unsigned char *>(&data), 8);
			}
		}
		return cnt;
	};

	auto ret = std::async(std::launch::async, prod1);

	auto res1 = std::async(std::launch::async, cons);
	auto res2 = std::async(std::launch::async, cons);
	auto res3 = std::async(std::launch::async, cons);
	ret.wait();
	auto V1 = res1.get();
	auto V2 = res2.get();
	auto V3 = res3.get();

	BOOST_CHECK_EQUAL(V1 + V2 + V3, 1'010'000);
}

BOOST_AUTO_TEST_CASE(multithreaded_tests3)
{
	std::unique_ptr<unsigned char[]> memblock(new unsigned char[1048576]);

	helics::ipc::detail::IpcBlockingPriorityQueueImpl queue(memblock.get(), 1048576);
	for (int64_t ii = 0; ii < 10'000; ++ii)
	{
		queue.push(reinterpret_cast<unsigned char *>(&ii), 8);
	}
	auto prod1 = [&]() {
		int64_t bdata;
		for (int64_t jj = 10'000; jj < 1'010'000; ++jj)
		{
			bdata = jj;
			queue.push(reinterpret_cast<unsigned char *>(&bdata), 8);
		}
	};

	auto cons = [&]() {
		int64_t data;
		auto res = queue.try_pop(reinterpret_cast<unsigned char *>(&data), 8);
		int64_t cnt = 0;
		while ((res))
		{
			++cnt;
			res = queue.try_pop(reinterpret_cast<unsigned char *>(&data), 8);
			if (res == 0)
			{  // make an additional sleep period so the producer can catch up
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				res = queue.try_pop(reinterpret_cast<unsigned char *>(&data), 8);
			}
		}
		return cnt;
	};


	auto ret1 = std::async(std::launch::async, prod1);
	auto ret2 = std::async(std::launch::async, prod1);
	auto ret3 = std::async(std::launch::async, prod1);

	auto res1 = std::async(std::launch::async, cons);
	auto res2 = std::async(std::launch::async, cons);
	auto res3 = std::async(std::launch::async, cons);
	ret1.wait();
	ret2.wait();
	ret3.wait();
	auto V1 = res1.get();
	auto V2 = res2.get();
	auto V3 = res3.get();

	BOOST_CHECK_EQUAL(V1 + V2 + V3, 3'010'000);
}

BOOST_AUTO_TEST_CASE(multithreaded_tests3_pop)
{
	std::unique_ptr<unsigned char[]> memblock(new unsigned char[1048576]);

	helics::ipc::detail::IpcBlockingPriorityQueueImpl queue(memblock.get(), 1048576);

	auto prod1 = [&]() {
		int64_t bdata;
		for (int64_t jj = 0; jj < 1'000'000; ++jj)
		{
			bdata = jj;
			queue.push(reinterpret_cast<unsigned char *>(&bdata), 8);
		}
		bdata = (-1);
		queue.push(reinterpret_cast<unsigned char *>(&bdata), 8);
	};

	auto cons = [&]() {
		int64_t data=0;
		int64_t cnt = 0;
		while (data>=0)
		{
			++cnt;
			queue.pop(reinterpret_cast<unsigned char *>(&data), 8);
		}
		return cnt;
	};


	auto ret1 = std::async(std::launch::async, prod1);
	auto ret2 = std::async(std::launch::async, prod1);
	auto ret3 = std::async(std::launch::async, prod1);

	auto res1 = std::async(std::launch::async, cons);
	auto res2 = std::async(std::launch::async, cons);
	auto res3 = std::async(std::launch::async, cons);
	ret1.wait();
	ret2.wait();
	ret3.wait();
	auto V1 = res1.get();
	auto V2 = res2.get();
	auto V3 = res3.get();

	BOOST_CHECK_EQUAL(V1 + V2 + V3, 3'000'003);
}


BOOST_AUTO_TEST_CASE(push_full_mem)
{
	std::unique_ptr<unsigned char[]> memblock(new unsigned char[4096]);

	std::unique_ptr<unsigned char[]> memblock2(new unsigned char[4096]);

	auto *queue =new(memblock2.get()) helics::ipc::detail::IpcBlockingPriorityQueueImpl(memblock.get(), 4096);
	std::vector<unsigned char> data(500, 'a');
	BOOST_CHECK(queue->try_push(data.data(), 420));  // this would go into the pull
	BOOST_CHECK(queue->try_push(data.data(), 420));  // this would go into the push
	BOOST_CHECK(queue->try_push(data.data(), 420));  // this would go into the push
	BOOST_CHECK(queue->try_push(data.data(), 420));  // this would go into the push

	BOOST_CHECK(!queue->try_push(data.data(), 420));  // this should fail as it is full
	BOOST_CHECK_EQUAL(queue->push(std::chrono::milliseconds(50), data.data(), 420),
		0);  // this should return 0 as timeout
}



BOOST_AUTO_TEST_CASE(multithreaded_tests3_pop_mem)
{
	std::unique_ptr<unsigned char[]> memblock(new unsigned char[1048576]);

	std::unique_ptr<unsigned char[]> memblock2(new unsigned char[4096]);

	auto *qn = new(memblock2.get()) helics::ipc::detail::IpcBlockingPriorityQueueImpl(memblock.get(), 1048576);

	auto prod1 = [&](void *data) {
		auto *queue = reinterpret_cast<helics::ipc::detail::IpcBlockingPriorityQueueImpl *>(data);
		int64_t bdata;
		for (int64_t jj = 0; jj < 1'000'000; ++jj)
		{
			bdata = jj;
			queue->push(reinterpret_cast<unsigned char *>(&bdata), 8);
		}
		bdata = (-1);
		queue->push(reinterpret_cast<unsigned char *>(&bdata), 8);
	};

	auto cons = [&](void *mem) {
		auto *queue = reinterpret_cast<helics::ipc::detail::IpcBlockingPriorityQueueImpl *>(mem);
		int64_t data = 0;
		int64_t cnt = 0;
		while (data >= 0)
		{
			++cnt;
			queue->pop(reinterpret_cast<unsigned char *>(&data), 8);
		}
		return cnt;
	};


	auto ret1 = std::async(std::launch::async, prod1, memblock2.get());
	auto ret2 = std::async(std::launch::async, prod1, memblock2.get());
	auto ret3 = std::async(std::launch::async, prod1, memblock2.get());

	auto res1 = std::async(std::launch::async, cons, memblock2.get());
	auto res2 = std::async(std::launch::async, cons, memblock2.get());
	auto res3 = std::async(std::launch::async, cons, memblock2.get());
	ret1.wait();
	ret2.wait();
	ret3.wait();
	auto V1 = res1.get();
	auto V2 = res2.get();
	auto V3 = res3.get();

	BOOST_CHECK_EQUAL(V1 + V2 + V3, 3'000'003);

}
*/
BOOST_AUTO_TEST_SUITE_END ()