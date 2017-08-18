/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/core/actionMessage.h"
#include <cstdio>

BOOST_AUTO_TEST_SUITE(actionMessage_tests)

using helics::Core;

using namespace helics;
/*
int32_t action_ = action_t::cmd_ignore; //4 -- command
public:
	int32_t source_id = 0;		//8 -- for federate_id or route_id
	int32_t source_handle = 0;	//12 -- for local handle or local code
	int32_t dest_id = 0;	//16 fed_id for a targeted message
	int32_t dest_handle = 0; //20 local handle for a targetted message
	bool iterationComplete = false; //24 
	bool required = false;  //!< flag indicating a publication is required
	bool error = false;		//!< flag indicating an error condition associated with the command
	bool flag = false;     //!< general flag for many purposes
	Time actionTime = timeZero;	//!< the time an action took place or will take place	//32
	std::string payload;		//!< string containing the data	//56 std::string is 24 bytes on most platforms
	std::string &name;  //!<alias payload to a name reference for registration functions
private:
	std::unique_ptr<AdditionalInfo> info_;   //!< pointer to an additional info structure with more data if required
*/
BOOST_AUTO_TEST_CASE(action_test1)
{
	helics::ActionMessage m(CMD_IGNORE);
	/*
	auto b = sizeof(m);
	BOOST_CHECK_LT(b, 64);
	if (b > 64)
	{
		printf("sizeof(info_)=%d\n", static_cast<int>(sizeof(std::unique_ptr<ActionMessage::AdditionalInfo>)));
		printf("payload %d\n", static_cast<int>(reinterpret_cast<char *>(&(m.info_)) - reinterpret_cast<char *>(&m)));
	}
	*/
	m.actionTime = 47.2342;
	m.payload = "this is a string that is sufficiently long";
	m.source_handle = 4;
	m.source_id = 232324;
	m.dest_id = 22552215;
	m.dest_handle = 2322342;

	std::string data;
	m.to_string(data);

	ActionMessage fr;
	fr.from_string(data);
	BOOST_CHECK(m.action() == fr.action());
	BOOST_CHECK_EQUAL(m.payload, fr.payload);
	BOOST_CHECK_EQUAL(m.source_handle, fr.source_handle);
	BOOST_CHECK_EQUAL(m.source_id, fr.source_id);
	BOOST_CHECK_EQUAL(m.dest_handle, fr.dest_handle);
	BOOST_CHECK_EQUAL(m.dest_id, fr.dest_id);
	BOOST_CHECK(m.actionTime = fr.actionTime);
}

BOOST_AUTO_TEST_CASE(action_test2)
{
	helics::ActionMessage m(CMD_REG_SUB);
	/*
	auto b = sizeof(m);
	BOOST_CHECK_LT(b, 64);
	if (b > 64)
	{
	printf("sizeof(info_)=%d\n", static_cast<int>(sizeof(std::unique_ptr<ActionMessage::AdditionalInfo>)));
	printf("payload %d\n", static_cast<int>(reinterpret_cast<char *>(&(m.info_)) - reinterpret_cast<char *>(&m)));
	}
	*/
	m.actionTime = 47.2342;
	m.payload = "this is a string that is sufficiently long";
	m.source_handle = 4;
	m.source_id = 232324;
	m.dest_id = 22552215;
	m.dest_handle = 2322342;
	m.info().source = "this is a message source of something else";
	m.info().orig_source="this is a different message source of something else";
	m.info().target = "a message target";
	m.info().Tdemin = 2342342.23423;
	m.info().Te = 24.2;
	
	std::string data;
	m.to_string(data);

	ActionMessage fr;
	fr.from_string(data);
	BOOST_CHECK(m.action() == fr.action());
	BOOST_CHECK_EQUAL(m.payload, fr.payload);
	BOOST_CHECK_EQUAL(m.source_handle, fr.source_handle);
	BOOST_CHECK_EQUAL(m.source_id, fr.source_id);
	BOOST_CHECK_EQUAL(m.dest_handle, fr.dest_handle);
	BOOST_CHECK_EQUAL(m.dest_id, fr.dest_id);
	BOOST_CHECK(m.actionTime == fr.actionTime);
	BOOST_CHECK_EQUAL(m.info().source, fr.info().source);
	
	BOOST_CHECK_EQUAL(m.info().orig_source, fr.info().orig_source);
	BOOST_CHECK_EQUAL(m.info().target ,fr.info().target);
	BOOST_CHECK(m.info().Tdemin == fr.info().Tdemin);
	BOOST_CHECK(m.info().Te == fr.info().Te);

}


BOOST_AUTO_TEST_SUITE_END()