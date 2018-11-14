/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>

#include "helics/common/CircularBuffer.hpp"


namespace utf = boost::unit_test;
using namespace helics::common;

BOOST_AUTO_TEST_SUITE (CircularBuffer_tests, *utf::label ("ci"))




BOOST_AUTO_TEST_CASE (test_circularbuffraw_simple)
{
    
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

BOOST_AUTO_TEST_CASE (test_circularbuffraw_loop_around)
{
    unsigned char *block = new unsigned char[1024];
    CircularBufferRaw buf (block, 1024);

    std::vector<unsigned char> testData (256, 'a');
    
    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);
    bool pushed=buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);
    pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (!pushed);

	BOOST_CHECK (!buf.isSpaceAvailable (20));
    int res = buf.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 200);
    BOOST_CHECK (buf.isSpaceAvailable (20));
    pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);

   buf.clear ();
    BOOST_CHECK (buf.empty ());
    delete[] block;
}


BOOST_AUTO_TEST_CASE (test_circularbuffraw_loop_around_repeat)
{
    unsigned char *block = new unsigned char[1520]; //3x504+4  otherwise there is a potential scenario in which 2 500byte messages cannot fit
    CircularBufferRaw buf (block, 1520);

    std::vector<unsigned char> testData (500, 'a');
	for (int ii = 1; ii <= 500;++ii)
	{
        bool pushed=buf.push (testData.data (), ii);
        BOOST_CHECK (pushed);
        pushed=buf.push (testData.data (), ii);
        BOOST_CHECK (pushed);
        int res = buf.pop (testData.data (), 500);
        BOOST_CHECK_EQUAL (res, ii);
        res = buf.pop (testData.data (), 500);
        BOOST_CHECK_EQUAL (res, ii);
        BOOST_CHECK (buf.empty ());
	}
    
    delete[] block;
}



BOOST_AUTO_TEST_CASE (test_circularbuff_simple)
{
    CircularBuffer buf (1024);

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
}

BOOST_AUTO_TEST_CASE (test_circularbuff_loop_around)
{
    CircularBuffer buf (1024);

    std::vector<unsigned char> testData (256, 'a');

    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);
    bool pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);
    pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (!pushed);

    BOOST_CHECK (!buf.isSpaceAvailable (20));
    int res = buf.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 200);
    BOOST_CHECK (buf.isSpaceAvailable (20));
    pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);

    buf.clear ();
    BOOST_CHECK (buf.empty ());
}

BOOST_AUTO_TEST_CASE (test_circularbuff_loop_around_repeat)
{
    CircularBuffer buf (1520);

    std::vector<unsigned char> testData (500, 'a');
    for (int ii = 1; ii <= 500; ++ii)
    {
        bool pushed = buf.push (testData.data (), ii);
        BOOST_CHECK (pushed);
        pushed = buf.push (testData.data (), ii);
        BOOST_CHECK (pushed);
        int res = buf.pop (testData.data (), 500);
        BOOST_CHECK_EQUAL (res, ii);
        res = buf.pop (testData.data (), 500);
        BOOST_CHECK_EQUAL (res, ii);
        BOOST_CHECK (buf.empty ());
    }

}


BOOST_AUTO_TEST_CASE (test_circularbuff_simple_move)
{
    CircularBuffer buf (1024);

    std::vector<unsigned char> testData (256, 'a');
    int res = buf.pop (testData.data (), 256);
    BOOST_CHECK_EQUAL (res, 0);
    BOOST_CHECK (buf.empty ());

    bool pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);
    testData.assign (256, '\0');
    BOOST_CHECK (!buf.empty ());

	CircularBuffer buf2 (std::move (buf));
    res = buf2.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 200);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[126], 'a');
    BOOST_CHECK_EQUAL (testData[199], 'a');
    BOOST_CHECK_EQUAL (testData[200], 0);

    BOOST_CHECK (buf2.empty ());
}


BOOST_AUTO_TEST_CASE (test_circularbuff_simple_copy)
{
    CircularBuffer buf (1024);

    std::vector<unsigned char> testData (256, 'a');
    int res = buf.pop (testData.data (), 256);
    BOOST_CHECK_EQUAL (res, 0);
    BOOST_CHECK (buf.empty ());

    bool pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);
    testData.assign (256, '\0');
    BOOST_CHECK (!buf.empty ());

	CircularBuffer buf2 (buf);

    res = buf.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 200);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[126], 'a');
    BOOST_CHECK_EQUAL (testData[199], 'a');
    BOOST_CHECK_EQUAL (testData[200], 0);

    BOOST_CHECK (buf.empty ());

	BOOST_CHECK (!buf2.empty ());

	 res = buf2.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 200);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[126], 'a');
    BOOST_CHECK_EQUAL (testData[199], 'a');
    BOOST_CHECK_EQUAL (testData[200], 0);
}


BOOST_AUTO_TEST_CASE (test_circularbuff_simple_move_assignment)
{
    CircularBuffer buf (1024);

    std::vector<unsigned char> testData (256, 'a');
    int res = buf.pop (testData.data (), 256);
    BOOST_CHECK_EQUAL (res, 0);
    BOOST_CHECK (buf.empty ());

    bool pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);
    testData.assign (256, '\0');
    BOOST_CHECK (!buf.empty ());

    CircularBuffer buf2 (200);
    buf2.push (testData.data (), 10);
    BOOST_CHECK (pushed);


	buf2 = std::move (buf);
    res = buf2.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 200);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[126], 'a');
    BOOST_CHECK_EQUAL (testData[199], 'a');
    BOOST_CHECK_EQUAL (testData[200], 0);

    BOOST_CHECK (buf2.empty ());
}


BOOST_AUTO_TEST_CASE (test_circularbuff_simple_copy_assignment)
{
    CircularBuffer buf (1024);

    std::vector<unsigned char> testData (256, 'a');
    int res = buf.pop (testData.data (), 256);
    BOOST_CHECK_EQUAL (res, 0);
    BOOST_CHECK (buf.empty ());

    bool pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);
    testData.assign (256, '\0');
    BOOST_CHECK (!buf.empty ());

    CircularBuffer buf2 (200);
    buf2.push (testData.data (), 10);
    BOOST_CHECK (pushed);

    buf2 = buf;
    BOOST_CHECK_EQUAL (buf2.capacity (), 1024);

    res = buf2.pop (testData.data (), 1024);
    BOOST_CHECK_EQUAL (res, 200);
    BOOST_CHECK_EQUAL (testData[0], 'a');
    BOOST_CHECK_EQUAL (testData[126], 'a');
    BOOST_CHECK_EQUAL (testData[199], 'a');
    BOOST_CHECK_EQUAL (testData[200], 0);

    BOOST_CHECK (buf2.empty ());

	BOOST_CHECK (!buf.empty ());
}


BOOST_AUTO_TEST_CASE (test_circularbuff_resize)
{
    CircularBuffer buf (1024);

    std::vector<unsigned char> testData (256, 'a');

    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);
    
	buf.resize (2048);
    auto pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);
    pushed=buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);

	BOOST_CHECK_EQUAL (buf.capacity (), 2048);
}

BOOST_AUTO_TEST_CASE (test_circularbuff_resize_smaller)
{
    CircularBuffer buf (1024);

    std::vector<unsigned char> testData (256, 'a');

    buf.push (testData.data (), 200);
    buf.push (testData.data (), 200);

    buf.resize (450);
    auto pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (!pushed);
    int sz = buf.pop (testData.data (), 256);
    BOOST_CHECK_EQUAL (sz,200);
    pushed = buf.push (testData.data (), 200);
    BOOST_CHECK (pushed);

    BOOST_CHECK_EQUAL (buf.capacity (), 450);
}

BOOST_AUTO_TEST_CASE (test_circularbuff_resize_bigger_wrap)
{
    CircularBuffer buf (1024);

    std::vector<unsigned char> testData (256, 'a');

    buf.push (testData.data (), 200);
    buf.push (testData.data (), 201);
    buf.push (testData.data (), 202);
    buf.push (testData.data (), 203);
    
	buf.pop (testData.data (), 256);
    buf.pop (testData.data (), 256);
    buf.push (testData.data (), 204);

	BOOST_CHECK (!buf.isSpaceAvailable (200));
    buf.resize (2048);
    auto pushed = buf.push (testData.data (), 205);
    BOOST_CHECK (pushed);
    pushed = buf.push (testData.data (), 206);
    BOOST_CHECK (pushed);
    BOOST_CHECK_EQUAL (buf.pop (testData.data (), 256), 202);
    BOOST_CHECK_EQUAL (buf.pop (testData.data (), 256), 203);
    BOOST_CHECK_EQUAL (buf.pop (testData.data (), 256), 204);
    BOOST_CHECK_EQUAL (buf.pop (testData.data (), 256), 205);
    BOOST_CHECK_EQUAL (buf.pop (testData.data (), 256), 206);
    BOOST_CHECK_EQUAL (buf.capacity (), 2048);
}


BOOST_AUTO_TEST_CASE (test_circularbuff_resize_smaller_wrap)
{
    CircularBuffer buf (1024);

    std::vector<unsigned char> testData (256, 'a');

    buf.push (testData.data (), 200);
    buf.push (testData.data (), 201);
    buf.push (testData.data (), 202);
    buf.push (testData.data (), 203);  

    buf.pop (testData.data (), 256);
    buf.pop (testData.data (), 256);
    buf.push (testData.data (), 204);
    buf.pop (testData.data (), 256);
    BOOST_CHECK (buf.isSpaceAvailable (205));
    buf.resize (620); //a size that can work
    BOOST_CHECK (!buf.isSpaceAvailable (205));
    auto pushed = buf.push (testData.data (), 205);
    BOOST_CHECK (!pushed);

	BOOST_CHECK_THROW (buf.resize (200), std::runtime_error);
    BOOST_CHECK_EQUAL (buf.pop (testData.data (), 256), 203);
    BOOST_CHECK_EQUAL (buf.pop (testData.data (), 256), 204);
    BOOST_CHECK_EQUAL (buf.capacity (), 620);
}


BOOST_AUTO_TEST_CASE (test_circularbuff_loop_around_repeat_resize)
{
    CircularBuffer buf (45);

    std::vector<unsigned char> testData (10000, 'a');
    for (int ii = 1; ii <= 10000; ++ii)
    {
		
        buf.resize (3 * (ii + 8));
        int res = buf.pop (testData.data (), 10000);
        BOOST_CHECK_EQUAL (res, ii - 1);
        bool pushed = buf.push (testData.data (), ii);
        BOOST_CHECK (pushed);
        pushed = buf.push (testData.data (), ii);
        BOOST_CHECK (pushed);
        res = buf.pop (testData.data (),10000);
        BOOST_CHECK_EQUAL (res, ii);
        res = buf.pop (testData.data (), 10000);
        BOOST_CHECK_EQUAL (res, ii);
        BOOST_CHECK (buf.empty ());
        pushed = buf.push (testData.data (), ii);
        BOOST_CHECK (pushed);
    }
}
BOOST_AUTO_TEST_SUITE_END ()