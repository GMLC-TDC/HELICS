/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>


/** these test cases test data_block and data_view objects
*/


#include "helics/application_api/Message.h"

BOOST_AUTO_TEST_SUITE(data_view_tests)

using namespace helics;

BOOST_AUTO_TEST_CASE(simple_data_view_tests)
{
	data_view dv("string");

	BOOST_CHECK_EQUAL(dv.string(), "string");

	std::string hip="hippo";
	data_view dv2(hip);
	BOOST_CHECK_EQUAL(dv2.string(), "hippo");
	BOOST_CHECK_EQUAL(dv2.size(), hip.length());
}

BOOST_AUTO_TEST_CASE(data_view_constructor_tests)
{


	const char *str = "this is a test string";

	data_view dv2(str);
	BOOST_CHECK_EQUAL(dv2.size(), strlen(str));
	BOOST_CHECK_EQUAL(dv2.string(), str);

	data_view dv3(str, 7);
	BOOST_CHECK_EQUAL(dv3.size(), 7);
	BOOST_CHECK_EQUAL(dv3.string(), "this is");

	stx::string_view stv(str, 10);
	//test copy constructor
	data_view db6(stv);
	BOOST_CHECK_EQUAL(db6.size(), stv.size());
	BOOST_CHECK_EQUAL(db6[8], stv[8]);

	//build from a vector
	std::vector<char> cvector(23, 'd');
	data_view db7(cvector);
	BOOST_CHECK_EQUAL(db7.size(), 23);
	BOOST_CHECK_EQUAL(db7[17], 'd');

	std::vector<double> dvector(10, 0.07);
	data_view db8(dvector);
	BOOST_CHECK_EQUAL(db8.size(), sizeof(double) * 10);


}


BOOST_AUTO_TEST_CASE(data_view_assignment_tests)
{
	data_block db(3, 't');

	data_view dv1(db);
	const char *str = "this is a test string";
	BOOST_CHECK_EQUAL(dv1.size(), 3);
	dv1 = str;
	BOOST_CHECK_EQUAL(dv1.size(), strlen(str));


	//assign the partial string
	data_block db3(str, 7);

	data_block db4(400, 'r');
	data_view dv4(db4);
	data_view dv5;
	//test move constructor
	dv5 = std::move(dv4);
	BOOST_CHECK_EQUAL(dv5.size(), 400);

}

BOOST_AUTO_TEST_CASE(data_view_range_for_ops)
{

	data_block test1(300, 25);
	data_view testv1(test1);
	
	int sum = 0;
	for (const auto te : testv1)
	{
		sum += te;
	}
	BOOST_CHECK_EQUAL(sum, 300 * 25);
}



/** test the swap function*/
BOOST_AUTO_TEST_CASE(data_view_swap)
{

	data_block test1(300, 23);

	data_view v1(test1);
	data_block test2(100, 45);
	data_view v2(test2);
	BOOST_CHECK_EQUAL(v1.size(), 300);
	BOOST_CHECK_EQUAL(v2.size(), 100);
	std::swap(v1, v2);
	BOOST_CHECK_EQUAL(v1.size(), 100);
	BOOST_CHECK_EQUAL(v2.size(), 300);
}



/** test the swap function*/
BOOST_AUTO_TEST_CASE(data_view_shared_ptr)
{
	auto db=std::make_shared<data_block>(400, 'r');
	data_view dv1(db);

	auto sz1 = db->size();
	auto checkel = (*db)[67];
	BOOST_CHECK_EQUAL(dv1.size(), sz1);
	BOOST_CHECK_EQUAL(dv1[67],checkel );

	BOOST_CHECK_EQUAL(db.use_count(), 2);
	db = nullptr;
	//should keep a valid memory
	BOOST_CHECK_EQUAL(dv1.size(), sz1);
	BOOST_CHECK_EQUAL(dv1[67], checkel);
}

BOOST_AUTO_TEST_SUITE_END()