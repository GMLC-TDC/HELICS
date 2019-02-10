/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

/** these test cases test data_block and data_view objects
 */

#include "helics/core/core-data.hpp"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (data_block_tests, *utf::label ("ci"))

using namespace helics;
BOOST_AUTO_TEST_CASE (simple_data_block_tests)
{
    data_block db (7);
    BOOST_CHECK_EQUAL (db.size (), 7u);
    // test direct assignment
    db[0] = 'h';
    db[1] = 'a';
    db[2] = 'p';
    db[3] = 'p';
    db[4] = 'y';
    db[5] = '!';
    db[6] = '!';
    BOOST_CHECK_EQUAL (db[3], 'p');
    BOOST_CHECK_EQUAL (db.to_string (), "happy!!");

    data_block db2 ("test_string");
    BOOST_CHECK_EQUAL (db2.to_string (), "test_string");
}

BOOST_AUTO_TEST_CASE (data_block_constructor_tests)
{
    data_block db (3, 't');
    BOOST_CHECK_EQUAL (db.size (), 3u);

    BOOST_CHECK_EQUAL (db.to_string (), "ttt");

    const char *str = "this is a test string";

    data_block db2 (str);
    BOOST_CHECK_EQUAL (db2.size (), strlen (str));
    BOOST_CHECK_EQUAL (db2.to_string (), str);

    data_block db3 (str, 7);
    BOOST_CHECK_EQUAL (db3.size (), 7u);
    BOOST_CHECK_EQUAL (db3.to_string (), "this is");

    data_block db4 (400, 'r');
    // test move constructor
    data_block db5 (std::move (db4));
    BOOST_CHECK_EQUAL (db5.size (), 400u);
    BOOST_CHECK_EQUAL (db4.size (), 0u);
    // test copy constructor
    data_block db6 (db5);
    BOOST_CHECK_EQUAL (db6.size (), db5.size ());
    BOOST_CHECK_EQUAL (db6[324], db5[324]);
    BOOST_CHECK_EQUAL (db6[324], 'r');

    // build from a vector
    std::vector<char> cvector (23, 'd');
    data_block db7 (cvector);
    BOOST_CHECK_EQUAL (db7.size (), 23);
    BOOST_CHECK_EQUAL (db7[17], 'd');

    std::vector<double> dvector (10, 0.07);
    data_block db8 (dvector);
    BOOST_CHECK_EQUAL (db8.size (), sizeof (double) * 10);
}

BOOST_AUTO_TEST_CASE (data_block_assignment_tests)
{
    data_block db (3, 't');

    BOOST_CHECK_EQUAL (db.to_string (), "ttt");

    const char *str = "this is a test string";

    db = str;
    BOOST_CHECK_EQUAL (db.size (), strlen (str));

    // assign the partial string
    data_block db3 (str, 7);
    db3 = db;
    BOOST_CHECK (db3 == db);

    data_block db4 (400, 'r');

    data_block db5 (10, 'e');
    // test move constructor
    db5 = std::move (db4);
    BOOST_CHECK_EQUAL (db5.size (), 400);
    BOOST_CHECK_PREDICATE (std::not_equal_to<size_t> (), (db4.size ()) (400));
}

BOOST_AUTO_TEST_CASE (data_block_range_for_ops)
{
    data_block test1 (300, 23);

    for (auto &te : test1)
    {
        te += 2;
    }

    BOOST_CHECK_EQUAL (test1[221], 25);
    int sum = 0;
    for (const auto te : test1)
    {
        sum += te;
    }
    BOOST_CHECK_EQUAL (sum, 300 * 25);
}

/** test the raw data pointer*/
BOOST_AUTO_TEST_CASE (data_block_data)
{
    data_block test1 (300, 23);

    auto rawdata = test1.data ();
    rawdata[45] = '\221';

    BOOST_CHECK_EQUAL (test1[45], '\221');
}

/** test the swap function*/
BOOST_AUTO_TEST_CASE (data_block_swap)
{
    data_block test1 (300, 23);

    data_block test2 (100, 45);

    std::swap (test1, test2);
    BOOST_CHECK_EQUAL (test1.size (), 100);
    BOOST_CHECK_EQUAL (test2.size (), 300);
}

BOOST_AUTO_TEST_SUITE_END ()
