/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>

#include "helics/core/ipc/IpcBlockingPriorityQueueImpl.hpp"


namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;


BOOST_AUTO_TEST_SUITE (IpcQueue_tests, *utf::label ("ci"))


BOOST_AUTO_TEST_CASE (test_stackqueueraw_simple)
{ 
	using namespace helics::ipc::detail;
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
    using namespace helics::ipc::detail;
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
    using namespace helics::ipc::detail;
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
    using namespace helics::ipc::detail;
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



BOOST_AUTO_TEST_CASE (test_circularbuffraw_simple)
{
    using namespace helics::ipc::detail;
    unsigned char *block = new unsigned char[1024];
    CircularBufferRaw buf (block, 1024);

    std::vector<unsigned char> testData (256, 'a');
    int res = buf.pop (testData.data (), 256);
    BOOST_CHECK_EQUAL (res, 0);
    BOOST_CHECK (buf.empty ());

    bool pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);
    testData.assign (256, '\0');
    BOOST_CHECK (!buf.empty ());
    res = buf.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 200);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[126], 'a');
    BOOST_CHECK_EQUAL (testData[199], 'a');
    BOOST_CHECK_EQUAL (testData[200], 0);

	 BOOST_CHECK (buf.empty ());
    delete[] block;
}

BOOST_AUTO_TEST_SUITE_END ()