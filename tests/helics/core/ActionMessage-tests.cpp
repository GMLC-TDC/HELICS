/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>

#include "helics/core/ActionMessage.hpp"
#include "helics/core/flagOperations.hpp"
#include <cstdio>
#include <set>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (ActionMessage_tests, *utf::label ("ci"))

using namespace helics;

BOOST_AUTO_TEST_CASE (action_test_to_string_conversion)
{
    helics::ActionMessage m (CMD_IGNORE);
    /*
    auto b = sizeof(m);
    BOOST_CHECK_LT(b, 64);
    if (b > 64)
    {
        printf("sizeof(extraInfo)=%d\n", static_cast<int>(sizeof(std::unique_ptr<ActionMessage::AdditionalInfo>)));
        printf("payload %d\n", static_cast<int>(reinterpret_cast<char *>(&(m.extraInfo)) - reinterpret_cast<char
    *>(&m)));
    }
    */
    m.actionTime = 47.2342;
    m.payload = "this is a string that is sufficiently long";
    m.source_handle = interface_handle{4};
    m.source_id = global_federate_id{232324};
    m.dest_id = global_federate_id{22552215};
    m.dest_handle = interface_handle{2322342};

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
}

BOOST_AUTO_TEST_CASE (action_test_to_string_conversion_info)
{
    helics::ActionMessage m (CMD_REG_INPUT);
    /*
    auto b = sizeof(m);
    BOOST_CHECK_LT(b, 64);
    if (b > 64)
    {
    printf("sizeof(extraInfo)=%d\n", static_cast<int>(sizeof(std::unique_ptr<ActionMessage::AdditionalInfo>)));
    printf("payload %d\n", static_cast<int>(reinterpret_cast<char *>(&(m.extraInfo)) - reinterpret_cast<char
    *>(&m)));
    }
    */
    m.actionTime = 47.2342;
    m.payload = "this is a string that is sufficiently long";
    m.source_handle = interface_handle (4);
    m.source_id = global_federate_id (232324);
    m.dest_id = global_federate_id (22552215);
    m.dest_handle = interface_handle (2322342);

    m.setString (sourceStringLoc, "this is a long source string to test");
    m.setString (origSourceStringLoc, "this is a longer alternate source string to test");
    m.setString (targetStringLoc, "this is a target");

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

    BOOST_CHECK (m.getStringData () == fr.getStringData ());
}

BOOST_AUTO_TEST_CASE (action_test_to_string_conversion_info2)
{
    helics::ActionMessage m (CMD_TIME_REQUEST);
    /*
    auto b = sizeof(m);
    BOOST_CHECK_LT(b, 64);
    if (b > 64)
    {
    printf("sizeof(extraInfo)=%d\n", static_cast<int>(sizeof(std::unique_ptr<ActionMessage::AdditionalInfo>)));
    printf("payload %d\n", static_cast<int>(reinterpret_cast<char *>(&(m.extraInfo)) - reinterpret_cast<char
    *>(&m)));
    }
    */
    m.actionTime = 47.2342;
    m.payload = "this is a string that is sufficiently long";
    m.source_handle = interface_handle{4};
    m.source_id = global_federate_id{232324};
    m.dest_id = global_federate_id{22552215};
    m.dest_handle = interface_handle{2322342};

    m.setString (sourceStringLoc, "this is a long source string to test");
    m.setString (origSourceStringLoc, "this is a longer alternate source string to test");
    m.setString (targetStringLoc, "this is a target");
    m.Tdemin = 2342532.2342;
    m.Tso = 54.7814;
    m.Te = Time::epsilon ();

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

    BOOST_CHECK (m.getStringData () == fr.getStringData ());
    BOOST_CHECK (m.Tso == fr.Tso);
    BOOST_CHECK (m.Te == fr.Te);
    BOOST_CHECK (m.Tdemin == fr.Tdemin);
}

BOOST_AUTO_TEST_CASE (constructor_test)
{
    // Default constructor
    helics::ActionMessage cmd;
    BOOST_CHECK (cmd.action () == helics::CMD_IGNORE);
    BOOST_CHECK_EQUAL (cmd.source_id, parent_broker_id);
    BOOST_CHECK (!cmd.source_handle.isValid ());
    BOOST_CHECK_EQUAL (cmd.dest_id, parent_broker_id);
    BOOST_CHECK (!cmd.dest_handle.isValid ());
    BOOST_CHECK_EQUAL (cmd.counter, 0);
    BOOST_CHECK_EQUAL (cmd.flags, 0);
    BOOST_CHECK_EQUAL (cmd.actionTime, helics::Time::zeroVal ());
    BOOST_CHECK (cmd.payload.empty ());

    // Additional info defaults
    BOOST_CHECK_EQUAL (cmd.Te, helics::timeZero);
    BOOST_CHECK_EQUAL (cmd.Tdemin, helics::timeZero);
    BOOST_CHECK (cmd.getStringData ().empty ());

    // Action constructor
    helics::ActionMessage cmd2 (helics::CMD_INIT);
    BOOST_CHECK (cmd2.action () == helics::CMD_INIT);
}

BOOST_AUTO_TEST_CASE (copy_constructor_test)
{
    helics::ActionMessage cmd (helics::CMD_INIT);
    cmd.source_id = global_federate_id{1};
    cmd.source_handle = interface_handle{2};
    cmd.dest_id = global_federate_id{3};
    cmd.dest_handle = interface_handle{4};
    cmd.flags = 0x1a2F;  // this has no significance
    cmd.actionTime = helics::Time::maxVal ();
    cmd.payload = "hello world";

    cmd.Te = helics::Time::maxVal ();
    cmd.Tdemin = helics::Time::minVal ();
    cmd.setStringData ("target", "source", "original_source");

    // Check operator= override
    helics::ActionMessage cmd_copy (cmd);
    BOOST_CHECK (cmd_copy.action () == helics::CMD_INIT);
    BOOST_CHECK_EQUAL (cmd_copy.source_id.baseValue (), 1);
    BOOST_CHECK_EQUAL (cmd_copy.source_handle.baseValue (), 2);
    BOOST_CHECK_EQUAL (cmd_copy.dest_id.baseValue (), 3);
    BOOST_CHECK_EQUAL (cmd_copy.dest_handle.baseValue (), 4);
    BOOST_CHECK_EQUAL (cmd_copy.flags, 0x1a2F);
    BOOST_CHECK_EQUAL (cmd_copy.actionTime, helics::Time::maxVal ());
    BOOST_CHECK_EQUAL (cmd_copy.payload, "hello world");
    BOOST_CHECK_EQUAL (cmd_copy.name, "hello world");  // aliased to payload

    BOOST_CHECK_EQUAL (cmd_copy.Te, helics::Time::maxVal ());
    BOOST_CHECK_EQUAL (cmd_copy.Tdemin, helics::Time::minVal ());
    BOOST_CHECK_EQUAL (cmd_copy.getString (sourceStringLoc), "source");
    BOOST_CHECK_EQUAL (cmd_copy.getString (targetStringLoc), "target");
    BOOST_CHECK_EQUAL (cmd_copy.getString (origSourceStringLoc), "original_source");
}

BOOST_AUTO_TEST_CASE (assignment_test)
{
    helics::ActionMessage cmd (helics::CMD_INIT);
    cmd.source_id = global_federate_id (1);
    cmd.source_handle = interface_handle (2);
    cmd.dest_id = global_federate_id (3);
    cmd.dest_handle = interface_handle (4);
    setActionFlag (cmd, iteration_requested_flag);
    setActionFlag (cmd, required_flag);
    setActionFlag (cmd, error_flag);
    cmd.actionTime = helics::Time::maxVal ();
    cmd.payload = "hello world";

    cmd.Te = helics::Time::maxVal ();
    cmd.Tdemin = helics::Time::minVal ();
    cmd.setStringData ("target", "source", "original_source", "original_dest");

    // Check operator= override
    helics::ActionMessage cmd_assign = cmd;
    BOOST_CHECK (cmd_assign.action () == helics::CMD_INIT);
    BOOST_CHECK_EQUAL (cmd_assign.source_id.baseValue (), 1);
    BOOST_CHECK_EQUAL (cmd_assign.source_handle.baseValue (), 2);
    BOOST_CHECK_EQUAL (cmd_assign.dest_id.baseValue (), 3);
    BOOST_CHECK_EQUAL (cmd_assign.dest_handle.baseValue (), 4);
    BOOST_CHECK (checkActionFlag (cmd_assign, iteration_requested_flag));
    BOOST_CHECK (checkActionFlag (cmd_assign, required_flag));
    BOOST_CHECK (checkActionFlag (cmd_assign, error_flag));
    BOOST_CHECK_EQUAL (cmd_assign.actionTime, helics::Time::maxVal ());
    BOOST_CHECK_EQUAL (cmd_assign.payload, "hello world");
    BOOST_CHECK_EQUAL (cmd_assign.name, "hello world");  // aliased to payload

    BOOST_CHECK_EQUAL (cmd_assign.Te, helics::Time::maxVal ());
    BOOST_CHECK_EQUAL (cmd_assign.Tdemin, helics::Time::minVal ());
    BOOST_CHECK_EQUAL (cmd_assign.getString (sourceStringLoc), "source");
    BOOST_CHECK_EQUAL (cmd_assign.getString (targetStringLoc), "target");
    BOOST_CHECK_EQUAL (cmd_assign.getString (origSourceStringLoc), "original_source");
    BOOST_CHECK_EQUAL (cmd_assign.getString (origDestStringLoc), "original_dest");
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

    BOOST_CHECK (cmd1 < cmd2);
    BOOST_CHECK (cmd2 < cmd3);
    BOOST_CHECK (cmd1 < cmd3);
    BOOST_CHECK (!(cmd1 < cmd1));

    // Insert messages into a set, check for correct ordering when scanned
    std::set<helics::ActionMessage> q;
    q.insert (cmd2);
    q.insert (cmd3);
    q.insert (cmd1);
    helics::Time ctime = helics::Time::minVal ();
    for (const auto &cmd : q)
    {
        BOOST_CHECK (cmd.actionTime >= ctime);
        ctime = cmd.actionTime;
    }
}

BOOST_AUTO_TEST_CASE (conversion_test)
{
    helics::ActionMessage cmd (helics::CMD_SEND_MESSAGE);
    cmd.source_id = global_federate_id (1);
    cmd.source_handle = interface_handle (2);
    cmd.dest_id = global_federate_id (3);
    cmd.dest_handle = interface_handle (4);
    setActionFlag (cmd, iteration_requested_flag);
    setActionFlag (cmd, required_flag);
    setActionFlag (cmd, error_flag);
    cmd.actionTime = 45.7;
    cmd.payload = std::string (5000, 'a');

    cmd.setStringData ("target", "source as a very long string test .........", "original_source");

    auto cmdString = cmd.to_string ();

    helics::ActionMessage cmd2 (cmdString);
    BOOST_CHECK (cmd.action () == cmd2.action ());
    BOOST_CHECK_EQUAL (cmd.actionTime, cmd2.actionTime);
    BOOST_CHECK_EQUAL (cmd.source_id, cmd2.source_id);
    BOOST_CHECK_EQUAL (cmd.dest_id, cmd2.dest_id);
    BOOST_CHECK_EQUAL (cmd.source_handle, cmd2.source_handle);
    BOOST_CHECK_EQUAL (cmd.dest_handle, cmd2.dest_handle);
    BOOST_CHECK_EQUAL (cmd.payload, cmd2.payload);
    BOOST_CHECK_EQUAL (cmd.flags, cmd2.flags);
    BOOST_CHECK (cmd.getStringData () == cmd2.getStringData ());
}

BOOST_AUTO_TEST_CASE (conversion_test2)
{
    helics::ActionMessage cmd (helics::CMD_SEND_MESSAGE);
    cmd.source_id = global_federate_id{1};
    cmd.source_handle = interface_handle{2};
    cmd.dest_id = global_federate_id{3};
    cmd.dest_handle = interface_handle{4};
    setActionFlag (cmd, iteration_requested_flag);
    setActionFlag (cmd, required_flag);
    setActionFlag (cmd, error_flag);
    cmd.actionTime = 45.7;
    cmd.payload = std::string (500000, 'j');

    cmd.setStringData ("target", "source as a very long string test .........", "original_source");

    auto cmdString = cmd.to_string ();

    helics::ActionMessage cmd2 (cmdString);
    BOOST_CHECK (cmd.action () == cmd2.action ());
    BOOST_CHECK_EQUAL (cmd.actionTime, cmd2.actionTime);
    BOOST_CHECK_EQUAL (cmd.source_id, cmd2.source_id);
    BOOST_CHECK_EQUAL (cmd.dest_id, cmd2.dest_id);
    BOOST_CHECK_EQUAL (cmd.source_handle, cmd2.source_handle);
    BOOST_CHECK_EQUAL (cmd.dest_handle, cmd2.dest_handle);
    BOOST_CHECK_EQUAL (cmd.payload, cmd2.payload);
    BOOST_CHECK_EQUAL (cmd.flags, cmd2.flags);
    BOOST_CHECK (cmd.getStringData () == cmd2.getStringData ());
}

BOOST_AUTO_TEST_CASE (message_message_conversion_test)
{
    helics::ActionMessage cmd (helics::CMD_SEND_MESSAGE);
    cmd.source_id = global_federate_id{1};
    cmd.source_handle = interface_handle{2};
    cmd.dest_id = global_federate_id{3};
    cmd.dest_handle = interface_handle{4};
    setActionFlag (cmd, iteration_requested_flag);
    setActionFlag (cmd, required_flag);
    setActionFlag (cmd, error_flag);
    cmd.actionTime = 45.7;
    cmd.payload = "hello world";

    cmd.setStringData ("target", "source as a very long string test .........", "original_source");

    auto msg = helics::createMessageFromCommand (cmd);

    BOOST_CHECK_EQUAL (cmd.actionTime, msg->time);
    BOOST_CHECK_EQUAL (cmd.getString (sourceStringLoc), msg->source);
    BOOST_CHECK_EQUAL (cmd.getString (origSourceStringLoc), msg->original_source);
    BOOST_CHECK_EQUAL (cmd.getString (targetStringLoc), msg->dest);
    BOOST_CHECK_EQUAL (cmd.payload, msg->data.to_string ());

    ActionMessage cmd2;
    cmd2.moveInfo (std::move (msg));
    BOOST_CHECK (cmd.action () == CMD_SEND_MESSAGE);
    BOOST_CHECK_EQUAL (cmd.actionTime, cmd2.actionTime);
    BOOST_CHECK_EQUAL (cmd.getString (0), cmd2.getString (0));
    BOOST_CHECK_EQUAL (cmd.getString (1), cmd2.getString (1));
    BOOST_CHECK_EQUAL (cmd.getString (2), cmd2.getString (2));
    BOOST_CHECK_EQUAL (cmd.payload, cmd.payload);
}

// check some error handling in the toByteArray function
BOOST_AUTO_TEST_CASE (check_conversions)
{
    helics::ActionMessage cmd (helics::CMD_PROTOCOL);
    cmd.messageID = 10;
    cmd.payload = "this is a payload test";

    auto cmdStr = cmd.to_string ();
    auto cmdVec = cmd.to_vector ();
    BOOST_CHECK_EQUAL (cmdStr.size (), cmdVec.size ());
    BOOST_CHECK_EQUAL (cmdStr, std::string (cmdVec.data (), cmdVec.size ()));

    auto testBuffer1 = std::make_unique<char[]> (cmdStr.size () + 20);
    auto testBuffer2 = std::make_unique<char[]> (cmdStr.size () >> 2);  // make a too small buffer

    auto res = cmd.toByteArray (testBuffer1.get (), static_cast<int> (cmdStr.size () + 20));
    BOOST_CHECK_EQUAL (res, cmdStr.size ());
    // just check to make sure the same string was written
    BOOST_CHECK_EQUAL (cmdStr, std::string (testBuffer1.get (), res));
    // this should return -1
    res = cmd.toByteArray (testBuffer2.get (), static_cast<int> (cmdStr.size () >> 2));
    BOOST_CHECK_EQUAL (res, -1);
}

// check some error handling in the toByteArray function
BOOST_AUTO_TEST_CASE (check_packetization)
{
    helics::ActionMessage cmd (helics::CMD_SEND_MESSAGE);
    cmd.source_id = global_federate_id (1);
    cmd.source_handle = interface_handle (2);
    cmd.dest_id = global_federate_id (3);
    cmd.dest_handle = interface_handle (4);
    setActionFlag (cmd, iteration_requested_flag);
    setActionFlag (cmd, required_flag);
    setActionFlag (cmd, error_flag);
    cmd.actionTime = 45.7;
    cmd.payload = "hello world";

    cmd.setStringData ("target", "source as a very long string test .........", "original_source");
    auto cmdStringNormal = cmd.to_string ();
    auto cmdString = cmd.packetize ();
    BOOST_CHECK_GE (cmdStringNormal.size () + 6, cmdString.size ());
    helics::ActionMessage cmd2;
    auto res = cmd2.depacketize (cmdString.data (), static_cast<int> (cmdString.size ()));
    BOOST_CHECK_EQUAL (res, cmdString.size ());
    BOOST_CHECK (cmd.action () == cmd2.action ());
    BOOST_CHECK_EQUAL (cmd.actionTime, cmd2.actionTime);
    BOOST_CHECK_EQUAL (cmd.source_id, cmd2.source_id);
    BOOST_CHECK_EQUAL (cmd.dest_id, cmd2.dest_id);
    BOOST_CHECK_EQUAL (cmd.source_handle, cmd2.source_handle);
    BOOST_CHECK_EQUAL (cmd.dest_handle, cmd2.dest_handle);
    BOOST_CHECK_EQUAL (cmd.payload, cmd2.payload);
    BOOST_CHECK_EQUAL (cmd.flags, cmd2.flags);
    BOOST_CHECK (cmd.getStringData () == cmd2.getStringData ());
}

BOOST_AUTO_TEST_SUITE_END ()
