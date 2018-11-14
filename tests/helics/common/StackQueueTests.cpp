/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>

#include "helics/core/ipc/IpcBlockingPriorityQueueImpl.hpp"


namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;
using namespace helics::common;

BOOST_AUTO_TEST_SUITE (StackQueue_tests, *utf::label ("ci"))


BOOST_AUTO_TEST_CASE (test_stackqueueraw_simple)
{ 
    unsigned char *block = new unsigned char[4096];
    StackQueueRaw stack (block, 4096);

	std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

	bool pushed = stack.push (testData.data (), 571);  
    BOOST_CHECK (pushed);
    testData.assign (1024, '\0');

	res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');
    BOOST_CHECK_EQUAL (testData[1023], 0);
	delete[] block;
}

BOOST_AUTO_TEST_CASE (test_stackqueueraw_3_push)
{
    unsigned char *block = new unsigned char[4096];
    StackQueueRaw stack (block, 4096);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'b');
    pushed = stack.push (testData.data (), 249);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'c');
    pushed = stack.push (testData.data (), 393);
    BOOST_CHECK (pushed);
    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 393);
    BOOST_CHECK_EQUAL (testData[0], 'c');
    BOOST_CHECK_EQUAL (testData[236], 'c');

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    BOOST_CHECK (stack.empty ());
    delete[] block;
}

BOOST_AUTO_TEST_CASE (test_stackqueueraw_push_full)
{
    unsigned char *block = new unsigned char[1024];
    StackQueueRaw stack (block, 1024);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'b');
    pushed = stack.push (testData.data (), 249);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'c');
    pushed = stack.push (testData.data (), 393);
    BOOST_CHECK (!pushed);

	BOOST_CHECK (!stack.isSpaceAvailable (393));
    BOOST_CHECK (!stack.isSpaceAvailable (200));
    BOOST_CHECK (stack.isSpaceAvailable (180));
    BOOST_CHECK_EQUAL (stack.getCurrentCount (), 2);
   
	pushed = stack.push (testData.data (), 180);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'd');
	BOOST_CHECK_EQUAL (stack.getCurrentCount (), 3);

	res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 180);
    BOOST_CHECK_EQUAL (testData[0], 'c');
    BOOST_CHECK_EQUAL (testData[179], 'c');
    BOOST_CHECK_EQUAL (testData[180], 'd'); //this is one past the copy

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

	res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    BOOST_CHECK (stack.empty ());
    delete[] block;
}


BOOST_AUTO_TEST_CASE (test_stackqueueraw_reverse)
{
    unsigned char *block = new unsigned char[4096];
    StackQueueRaw stack (block, 4096);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'b');
    pushed = stack.push (testData.data (), 249);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'c');
    pushed = stack.push (testData.data (), 393);
    BOOST_CHECK (pushed);

	stack.reverse ();

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

	 res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 393);
    BOOST_CHECK_EQUAL (testData[0], 'c');
    BOOST_CHECK_EQUAL (testData[236], 'c');


    BOOST_CHECK (stack.empty ());
    delete[] block;
}

BOOST_AUTO_TEST_CASE (test_stackqueue_simple)
{
    using namespace helics::ipc::detail;
    
    StackQueue stack (4096);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, '\0');

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');
    BOOST_CHECK_EQUAL (testData[1023], 0);
}

BOOST_AUTO_TEST_CASE (test_stackqueue_3_push)
{
    using namespace helics::ipc::detail;
    StackQueue stack (4096);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'b');
    pushed = stack.push (testData.data (), 249);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'c');
    pushed = stack.push (testData.data (), 393);
    BOOST_CHECK (pushed);
    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 393);
    BOOST_CHECK_EQUAL (testData[0], 'c');
    BOOST_CHECK_EQUAL (testData[236], 'c');

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    BOOST_CHECK (stack.empty ());
}

BOOST_AUTO_TEST_CASE (test_stackqueue_push_full)
{
    using namespace helics::ipc::detail;
    StackQueue stack (1024);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'b');
    pushed = stack.push (testData.data (), 249);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'c');
    pushed = stack.push (testData.data (), 393);
    BOOST_CHECK (!pushed);

    BOOST_CHECK (!stack.isSpaceAvailable (393));
    BOOST_CHECK (!stack.isSpaceAvailable (200));
    BOOST_CHECK (stack.isSpaceAvailable (180));
    BOOST_CHECK_EQUAL (stack.getCurrentCount (), 2);

    pushed = stack.push (testData.data (), 180);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'd');
    BOOST_CHECK_EQUAL (stack.getCurrentCount (), 3);

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 180);
    BOOST_CHECK_EQUAL (testData[0], 'c');
    BOOST_CHECK_EQUAL (testData[179], 'c');
    BOOST_CHECK_EQUAL (testData[180], 'd');  // this is one past the copy

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    BOOST_CHECK (stack.empty ());
}

BOOST_AUTO_TEST_CASE (test_stackqueue_reverse)
{
    using namespace helics::ipc::detail;
    StackQueue stack ( 4096);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'b');
    pushed = stack.push (testData.data (), 249);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'c');
    pushed = stack.push (testData.data (), 393);
    BOOST_CHECK (pushed);

    stack.reverse ();

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 393);
    BOOST_CHECK_EQUAL (testData[0], 'c');
    BOOST_CHECK_EQUAL (testData[236], 'c');

    BOOST_CHECK (stack.empty ());
}


BOOST_AUTO_TEST_CASE (test_stackqueue_move)
{
    using namespace helics::ipc::detail;

    StackQueue stack (2048);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    stack.push (testData.data (), 571);
    testData.assign (1024, '\0');

	StackQueue mstack (std::move (stack));
    BOOST_CHECK_EQUAL (mstack.getCurrentCount (), 1);
    res = mstack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');
    BOOST_CHECK_EQUAL (testData[1023], 0);
}


BOOST_AUTO_TEST_CASE (test_stackqueue_3_push_and_copy)
{
    using namespace helics::ipc::detail;
    StackQueue stack (4096);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'b');
    pushed = stack.push (testData.data (), 249);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'c');
    pushed = stack.push (testData.data (), 393);
    BOOST_CHECK (pushed);

	StackQueue cstack (stack);
    stack.reverse ();

    res = cstack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 393);
    BOOST_CHECK_EQUAL (testData[0], 'c');
    BOOST_CHECK_EQUAL (testData[236], 'c');

    res = cstack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

    res = cstack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    BOOST_CHECK (cstack.empty ());
	BOOST_CHECK (!stack.empty ());

	//check the original still has data and was reversed
	res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

	//now copy assign the stack
	cstack = stack;
    
	res = cstack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

}

BOOST_AUTO_TEST_CASE (test_stackqueue_move_assignement)
{
    using namespace helics::ipc::detail;

    StackQueue stack (2048);
    StackQueue stack2 (1024);
    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    stack.push (testData.data (), 571);
    testData.assign (1024, 'b');
    stack2.push (testData.data (), 397);

	stack2 = std::move (stack);
    testData.assign (1024, '\0');

    BOOST_CHECK_EQUAL (stack2.getCurrentCount (), 1);
    res = stack2.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');
    BOOST_CHECK_EQUAL (testData[1023], 0);
}


BOOST_AUTO_TEST_CASE (test_stackqueue_3_push_resize)
{
    using namespace helics::ipc::detail;
    StackQueue stack (2048);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'b');
    pushed = stack.push (testData.data (), 249);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'c');
    pushed = stack.push (testData.data (), 393);
    BOOST_CHECK (pushed);
	//make sure to trigger a reallocation in memory
	stack.resize (100000);
    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 393);
    BOOST_CHECK_EQUAL (testData[0], 'c');
    BOOST_CHECK_EQUAL (testData[236], 'c');

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    BOOST_CHECK (stack.empty ());
}



BOOST_AUTO_TEST_CASE (test_stackqueue_3_push_resize_shrink)
{
    using namespace helics::ipc::detail;
    StackQueue stack (2048);

    std::vector<unsigned char> testData (1024, 'a');
    int res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 0);

    bool pushed = stack.push (testData.data (), 571);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'b');
    pushed = stack.push (testData.data (), 249);
    BOOST_CHECK (pushed);
    testData.assign (1024, 'c');
    pushed = stack.push (testData.data (), 393);
    BOOST_CHECK (pushed);

    stack.resize (1400);

    BOOST_CHECK_EQUAL (stack.capacity (), 1400);

	BOOST_CHECK_THROW (stack.resize (95), std::runtime_error);
    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 393);
    BOOST_CHECK_EQUAL (testData[0], 'c');
    BOOST_CHECK_EQUAL (testData[236], 'c');

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 249);
    BOOST_CHECK_EQUAL (testData[0], 'b');
    BOOST_CHECK_EQUAL (testData[236], 'b');
    BOOST_CHECK (!stack.empty ());

    res = stack.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 571);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[236], 'a');
    BOOST_CHECK_EQUAL (testData[570], 'a');

    BOOST_CHECK (stack.empty ());
}

BOOST_AUTO_TEST_SUITE_END ()