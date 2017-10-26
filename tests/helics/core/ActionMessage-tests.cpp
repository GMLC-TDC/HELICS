/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/core/ActionMessage.h"
#include <cstdio>

BOOST_AUTO_TEST_SUITE (ActionMessage_tests)

using helics::Core;

using namespace helics;
/*
int32_t action_ = action_t::cmd_ignore; //4 -- command
public:
    int32_t source_id = 0;		//8 -- for federate_id or route_id
    int32_t source_handle = 0;	//12 -- for local handle or local code
    int32_t dest_id = 0;	//16 fed_id for a targeted message
    int32_t dest_handle = 0; //20 local handle for a targeted message
    bool iterationComplete = false; //24
    bool required = false;  //!< flag indicating a publication is required
    bool error = false;		//!< flag indicating an error condition associated with the command
    bool flag = false;     //!< general flag for many purposes
    Time actionTime = timeZero;	//!< the time an action took place or will take place	//32
    std::string payload;		//!< string containing the data	//56 std::string is 24 bytes on most platforms
    std::string &name;  //!<alias payload to a name reference for registration functions
private:
    std::unique_ptr<AdditionalInfo> info_;   //!< pointer to an additional info structure with more data if
required
*/
BOOST_AUTO_TEST_CASE (action_test_to_string_conversion)
{
    helics::ActionMessage m (CMD_IGNORE);
    /*
    auto b = sizeof(m);
    BOOST_CHECK_LT(b, 64);
    if (b > 64)
    {
        printf("sizeof(info_)=%d\n", static_cast<int>(sizeof(std::unique_ptr<ActionMessage::AdditionalInfo>)));
        printf("payload %d\n", static_cast<int>(reinterpret_cast<char *>(&(m.info_)) - reinterpret_cast<char
    *>(&m)));
    }
    */
    m.actionTime = 47.2342;
    m.payload = "this is a string that is sufficiently long";
    m.source_handle = 4;
    m.source_id = 232324;
    m.dest_id = 22552215;
    m.dest_handle = 2322342;

    std::string data;
    m.to_string (data);

    ActionMessage fr;
    fr.from_string (data);
    BOOST_CHECK (m.action () == fr.action ());
    BOOST_CHECK_EQUAL (m.payload, fr.payload);
    BOOST_CHECK_EQUAL (m.source_handle, fr.source_handle);
    BOOST_CHECK_EQUAL (m.source_id, fr.source_id);
    BOOST_CHECK_EQUAL (m.dest_handle, fr.dest_handle);
    BOOST_CHECK_EQUAL (m.dest_id, fr.dest_id);
    BOOST_CHECK (m.actionTime = fr.actionTime);
}

BOOST_AUTO_TEST_CASE (action_test_to_string_conversion_info)
{
    helics::ActionMessage m (CMD_REG_SUB);
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

    m.info ().source = "this is a long source string to test";
    m.info ().orig_source = "this is a longer alternate source string to test";
    m.info ().target = "this is a target";
    m.info ().Tdemin = 2342532.2342;
    m.info ().Te = Time::epsilon ();

    std::string data;
    m.to_string (data);

    ActionMessage fr;
    fr.from_string (data);
    BOOST_CHECK (m.action () == fr.action ());
    BOOST_CHECK_EQUAL (m.payload, fr.payload);
    BOOST_CHECK_EQUAL (m.source_handle, fr.source_handle);
    BOOST_CHECK_EQUAL (m.source_id, fr.source_id);
    BOOST_CHECK_EQUAL (m.dest_handle, fr.dest_handle);
    BOOST_CHECK_EQUAL (m.dest_id, fr.dest_id);
    BOOST_CHECK (m.actionTime == fr.actionTime);

    BOOST_CHECK_EQUAL (m.info ().source, fr.info ().source);
    BOOST_CHECK_EQUAL (m.info ().orig_source, fr.info ().orig_source);
    BOOST_CHECK_EQUAL (m.info ().target, fr.info ().target);
    BOOST_CHECK (m.info ().Te == fr.info ().Te);
    BOOST_CHECK (m.info ().Tdemin == fr.info ().Tdemin);
}

BOOST_AUTO_TEST_CASE (constructor_test)
{
    // Default constructor
    helics::ActionMessage cmd;
    BOOST_CHECK (cmd.action () == helics::CMD_IGNORE);
    BOOST_CHECK_EQUAL (cmd.source_id, 0);
    BOOST_CHECK_EQUAL (cmd.source_handle, 0);
    BOOST_CHECK_EQUAL (cmd.dest_id, 0);
    BOOST_CHECK_EQUAL (cmd.dest_handle, 0);
    BOOST_CHECK_EQUAL (cmd.iterationComplete, false);
    BOOST_CHECK_EQUAL (cmd.required, false);
    BOOST_CHECK_EQUAL (cmd.error, false);
    BOOST_CHECK_EQUAL (cmd.actionTime, helics::Time::zeroVal ());
    BOOST_CHECK (cmd.payload.empty ());

    // Additional info defaults
    BOOST_CHECK_EQUAL (cmd.info ().Te, helics::timeZero);
    BOOST_CHECK_EQUAL (cmd.info ().Tdemin, helics::timeZero);
    BOOST_CHECK (cmd.info ().source.empty ());
    BOOST_CHECK (cmd.info ().target.empty ());
    BOOST_CHECK (cmd.info ().type.empty ());
    BOOST_CHECK (cmd.info ().units.empty ());
    BOOST_CHECK (cmd.info ().orig_source.empty ());

    // Action constructor
    helics::ActionMessage cmd2 (helics::CMD_INIT);
    BOOST_CHECK (cmd2.action () == helics::CMD_INIT);
}

BOOST_AUTO_TEST_CASE (copy_constructor_test)
{
    helics::ActionMessage cmd (helics::CMD_INIT);
    cmd.source_id = 1;
    cmd.source_handle = 2;
    cmd.dest_id = 3;
    cmd.dest_handle = 4;
    cmd.iterationComplete = true;
    cmd.required = true;
    cmd.error = true;
    cmd.actionTime = helics::Time::maxVal ();
    cmd.payload = "hello world";

    cmd.info ().Te = helics::Time::maxVal ();
    cmd.info ().Tdemin = helics::Time::minVal ();
    cmd.info ().source = "source";  // type aliased to source
    cmd.info ().target = "target";  // units aliased to target
    cmd.info ().orig_source = "origsrc";

    // Check operator= override
    helics::ActionMessage cmd_copy (cmd);
    BOOST_CHECK (cmd_copy.action () == helics::CMD_INIT);
    BOOST_CHECK_EQUAL (cmd_copy.source_id, 1);
    BOOST_CHECK_EQUAL (cmd_copy.source_handle, 2);
    BOOST_CHECK_EQUAL (cmd_copy.dest_id, 3);
    BOOST_CHECK_EQUAL (cmd_copy.dest_handle, 4);
    BOOST_CHECK_EQUAL (cmd_copy.iterationComplete, true);
    BOOST_CHECK_EQUAL (cmd_copy.required, true);
    BOOST_CHECK_EQUAL (cmd_copy.error, true);
    BOOST_CHECK_EQUAL (cmd_copy.actionTime, helics::Time::maxVal ());
    BOOST_CHECK (cmd_copy.payload.compare ("hello world") == 0);
    BOOST_CHECK (cmd_copy.name.compare ("hello world") == 0);  // aliased to payload

    BOOST_CHECK_EQUAL (cmd_copy.info ().Te, helics::Time::maxVal ());
    BOOST_CHECK_EQUAL (cmd_copy.info ().Tdemin, helics::Time::minVal ());
    BOOST_CHECK (cmd_copy.info ().source.compare ("source") == 0);
    BOOST_CHECK (cmd_copy.info ().target.compare ("target") == 0);
    BOOST_CHECK (cmd_copy.info ().type.compare ("source") == 0);  // aliased to source
    BOOST_CHECK (cmd_copy.info ().units.compare ("target") == 0);  // aliased to target
    BOOST_CHECK (cmd_copy.info ().orig_source.compare ("origsrc") == 0);
}

BOOST_AUTO_TEST_CASE (assignment_test)
{
    helics::ActionMessage cmd (helics::CMD_INIT);
    cmd.source_id = 1;
    cmd.source_handle = 2;
    cmd.dest_id = 3;
    cmd.dest_handle = 4;
    cmd.iterationComplete = true;
    cmd.required = true;
    cmd.error = true;
    cmd.actionTime = helics::Time::maxVal ();
    cmd.payload = "hello world";

    cmd.info ().Te = helics::Time::maxVal ();
    cmd.info ().Tdemin = helics::Time::minVal ();
    cmd.info ().source = "source";  // type aliased to source
    cmd.info ().target = "target";  // units aliased to target
    cmd.info ().orig_source = "origsrc";

    // Check operator= override
    helics::ActionMessage cmd_assign = cmd;
    BOOST_CHECK (cmd_assign.action () == helics::CMD_INIT);
    BOOST_CHECK_EQUAL (cmd_assign.source_id, 1);
    BOOST_CHECK_EQUAL (cmd_assign.source_handle, 2);
    BOOST_CHECK_EQUAL (cmd_assign.dest_id, 3);
    BOOST_CHECK_EQUAL (cmd_assign.dest_handle, 4);
    BOOST_CHECK_EQUAL (cmd_assign.iterationComplete, true);
    BOOST_CHECK_EQUAL (cmd_assign.required, true);
    BOOST_CHECK_EQUAL (cmd_assign.error, true);
    BOOST_CHECK_EQUAL (cmd_assign.actionTime, helics::Time::maxVal ());
    BOOST_CHECK (cmd_assign.payload.compare ("hello world") == 0);
    BOOST_CHECK (cmd_assign.name.compare ("hello world") == 0);  // aliased to payload

    BOOST_CHECK_EQUAL (cmd_assign.info ().Te, helics::Time::maxVal ());
    BOOST_CHECK_EQUAL (cmd_assign.info ().Tdemin, helics::Time::minVal ());
    BOOST_CHECK (cmd_assign.info ().source.compare ("source") == 0);
    BOOST_CHECK (cmd_assign.info ().target.compare ("target") == 0);
    BOOST_CHECK (cmd_assign.info ().type.compare ("source") == 0);  // aliased to source
    BOOST_CHECK (cmd_assign.info ().units.compare ("target") == 0);  // aliased to target
    BOOST_CHECK (cmd_assign.info ().orig_source.compare ("origsrc") == 0);
}

BOOST_AUTO_TEST_CASE (comparison_test)
{
    helics::ActionMessage cmd1 (helics::CMD_INIT);
    cmd1.actionTime = helics::Time::minVal ();

    helics::ActionMessage cmd2 (helics::CMD_INIT);
    cmd2.actionTime = helics::Time::zeroVal ();

    helics::ActionMessage cmd3 (helics::CMD_INIT);
    cmd3.actionTime = helics::Time::maxVal ();

    // Check less than comparison (not implemented yet)
    /*
    BOOST_CHECK(cmd1 < cmd2);
    BOOST_CHECK(cmd2 < cmd3);
    BOOST_CHECK(cmd1 < cmd3);
    BOOST_CHECK_EQUAL(cmd1 < cmd1, false);
    */

    // Insert messages into a deque, check for correct ordering when removed
    std::deque<helics::ActionMessage> q;
    q.push_back (cmd2);
    q.push_back (cmd3);
    q.push_back (cmd1);
}

BOOST_AUTO_TEST_CASE (conversion_test)
{
    helics::ActionMessage cmd (helics::CMD_SEND_MESSAGE);
    cmd.source_id = 1;
    cmd.source_handle = 2;
    cmd.dest_id = 3;
    cmd.dest_handle = 4;
    cmd.iterationComplete = true;
    cmd.required = true;
    cmd.error = true;
    cmd.flag = false;
    cmd.actionTime = 45.7;
    cmd.payload = "hello world";

    cmd.info ().Te = 0.89;
    cmd.info ().Tdemin = 5.55;
    cmd.info ().source = "source as a very long string test .........";  // type aliased to source
    cmd.info ().target = "target";  // units aliased to target
    cmd.info ().orig_source = "origsrc";

    auto cmdString = cmd.to_string ();

    helics::ActionMessage cmd2 (cmdString);
    BOOST_CHECK (cmd.action () == cmd2.action ());
    BOOST_CHECK_EQUAL (cmd.actionTime, cmd2.actionTime);
    BOOST_CHECK_EQUAL (cmd.source_id, cmd2.source_id);
    BOOST_CHECK_EQUAL (cmd.dest_id, cmd2.dest_id);
    BOOST_CHECK_EQUAL (cmd.source_handle, cmd2.source_handle);
    BOOST_CHECK_EQUAL (cmd.dest_handle, cmd2.dest_handle);
    BOOST_CHECK_EQUAL (cmd.payload, cmd2.payload);
    BOOST_CHECK_EQUAL (cmd.iterationComplete, cmd2.iterationComplete);
    BOOST_CHECK_EQUAL (cmd.error, cmd2.error);
    BOOST_CHECK_EQUAL (cmd.required, cmd2.required);
    BOOST_CHECK_EQUAL (cmd.flag, cmd2.flag);
    BOOST_CHECK_EQUAL (cmd.info ().Te, cmd2.info ().Te);
    BOOST_CHECK_EQUAL (cmd.info ().Tdemin, cmd2.info ().Tdemin);
    BOOST_CHECK_EQUAL (cmd.info ().source, cmd2.info ().source);
    BOOST_CHECK_EQUAL (cmd.info ().target, cmd2.info ().target);
    BOOST_CHECK_EQUAL (cmd.info ().orig_source, cmd2.info ().orig_source);
}

BOOST_AUTO_TEST_CASE (message_conversion_test)
{
    helics::ActionMessage cmd (helics::CMD_SEND_MESSAGE);
    cmd.source_id = 1;
    cmd.source_handle = 2;
    cmd.dest_id = 3;
    cmd.dest_handle = 4;
    cmd.iterationComplete = true;
    cmd.required = true;
    cmd.error = true;
    cmd.flag = false;
    cmd.actionTime = 45.7;
    cmd.payload = "hello world";

    cmd.info ().Te = 0.89;
    cmd.info ().Tdemin = 5.55;
    cmd.info ().source = "source as a very long string test .........";  // type aliased to source
    cmd.info ().target = "target";  // units aliased to target
    cmd.info ().orig_source = "origsrc";

    auto msg = helics::createMessage (cmd);

    BOOST_CHECK_EQUAL (cmd.actionTime, msg->time);
    BOOST_CHECK_EQUAL (cmd.info ().source, msg->src);
    BOOST_CHECK_EQUAL (cmd.info ().orig_source, msg->origsrc);
    BOOST_CHECK_EQUAL (cmd.info ().target, msg->dest);
    BOOST_CHECK_EQUAL (cmd.payload, msg->data.to_string ());

    ActionMessage cmd2;
    cmd2.moveInfo (std::move (msg));
    BOOST_CHECK (cmd.action () == CMD_SEND_MESSAGE);
    BOOST_CHECK_EQUAL (cmd.actionTime, cmd2.actionTime);
    BOOST_CHECK_EQUAL (cmd.info ().source, cmd2.info ().source);
    BOOST_CHECK_EQUAL (cmd.info ().orig_source, cmd2.info ().orig_source);
    BOOST_CHECK_EQUAL (cmd.info ().target, cmd.info ().target);
    BOOST_CHECK_EQUAL (cmd.payload, cmd.payload);
}
BOOST_AUTO_TEST_SUITE_END ()