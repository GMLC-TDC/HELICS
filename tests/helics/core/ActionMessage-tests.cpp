/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/ActionMessage.hpp"
#include "helics/core/flagOperations.hpp"

#include "gtest/gtest.h"
#include <cstdio>
#include <set>

using namespace helics;

TEST(ActionMessage, action_test_to_string_conversion)
{
    helics::ActionMessage m(CMD_IGNORE);
    /*
    auto b = sizeof(m);
    BOOST_CHECK_LT(b, 64);
    if (b > 64)
    {
        printf("sizeof(extraInfo)=%d\n",
    static_cast<int>(sizeof(std::unique_ptr<ActionMessage::AdditionalInfo>))); printf("payload
    %d\n", static_cast<int>(reinterpret_cast<char *>(&(m.extraInfo)) - reinterpret_cast<char
    *>(&m)));
    }
    */
    m.actionTime = 47.2342;
    m.payload = "this is a string that is sufficiently long";
    m.source_handle = interface_handle{4};
    m.source_id = global_federate_id{232324};
    m.dest_id = global_federate_id{22552215};
    m.dest_handle = interface_handle{2322342};
    m.sequenceID = 987;
    std::string data;
    m.to_string(data);

    ActionMessage fr;
    fr.from_string(data);
    EXPECT_TRUE(m.action() == fr.action());
    EXPECT_EQ(m.payload, fr.payload);
    EXPECT_EQ(m.source_handle, fr.source_handle);
    EXPECT_EQ(m.source_id, fr.source_id);
    EXPECT_EQ(m.dest_handle, fr.dest_handle);
    EXPECT_EQ(m.dest_id, fr.dest_id);
    EXPECT_TRUE(m.actionTime == fr.actionTime);
    EXPECT_TRUE(m.sequenceID == fr.sequenceID);
}

TEST(ActionMessage, action_test_to_string_conversion_info)
{
    helics::ActionMessage m(CMD_REG_INPUT);
    /*
    auto b = sizeof(m);
    BOOST_CHECK_LT(b, 64);
    if (b > 64)
    {
    printf("sizeof(extraInfo)=%d\n",
    static_cast<int>(sizeof(std::unique_ptr<ActionMessage::AdditionalInfo>))); printf("payload
    %d\n", static_cast<int>(reinterpret_cast<char *>(&(m.extraInfo)) - reinterpret_cast<char
    *>(&m)));
    }
    */
    m.actionTime = 47.2342;
    m.payload = "this is a string that is sufficiently long";
    m.source_handle = interface_handle(4);
    m.source_id = global_federate_id(232324);
    m.dest_id = global_federate_id(22552215);
    m.dest_handle = interface_handle(2322342);
    m.sequenceID = 987;

    m.setString(sourceStringLoc, "this is a long source string to test");
    m.setString(origSourceStringLoc, "this is a longer alternate source string to test");
    m.setString(targetStringLoc, "this is a target");

    std::string data;
    m.to_string(data);

    ActionMessage fr;
    fr.from_string(data);
    EXPECT_TRUE(m.action() == fr.action());
    EXPECT_EQ(m.payload, fr.payload);
    EXPECT_EQ(m.source_handle, fr.source_handle);
    EXPECT_EQ(m.source_id, fr.source_id);
    EXPECT_EQ(m.dest_handle, fr.dest_handle);
    EXPECT_EQ(m.dest_id, fr.dest_id);
    EXPECT_TRUE(m.actionTime == fr.actionTime);
    EXPECT_EQ(m.sequenceID, fr.sequenceID);
    EXPECT_TRUE(m.getStringData() == fr.getStringData());
}

TEST(ActionMessage, action_test_to_string_time_request)
{
    helics::ActionMessage m(CMD_TIME_REQUEST);
    /*
    auto b = sizeof(m);
    BOOST_CHECK_LT(b, 64);
    if (b > 64)
    {
    printf("sizeof(extraInfo)=%d\n",
    static_cast<int>(sizeof(std::unique_ptr<ActionMessage::AdditionalInfo>))); printf("payload
    %d\n", static_cast<int>(reinterpret_cast<char *>(&(m.extraInfo)) - reinterpret_cast<char
    *>(&m)));
    }
    */
    m.payload = "this is a string that is sufficiently long";
    m.actionTime = 47.2342;
    m.source_handle = interface_handle{4};
    m.source_id = global_federate_id{232324};
    m.dest_id = global_federate_id{22552215};
    m.dest_handle = interface_handle{2322342};
    m.setString(sourceStringLoc, "this is a long source string to test");
    // this is a time request and the payload and strings don't get copied
    m.Tdemin = 2342532.2342;
    m.Tso = 54.7814;
    m.Te = Time::epsilon();

    std::string data;
    m.to_string(data);

    ActionMessage fr;
    fr.from_string(data);
    EXPECT_TRUE(m.action() == fr.action());
    EXPECT_TRUE(fr.payload.empty());
    EXPECT_EQ(m.source_handle, fr.source_handle);
    EXPECT_EQ(m.source_id, fr.source_id);
    EXPECT_EQ(m.dest_handle, fr.dest_handle);
    EXPECT_EQ(m.dest_id, fr.dest_id);
    EXPECT_TRUE(m.actionTime == fr.actionTime);

    EXPECT_TRUE(fr.getStringData().empty());
    EXPECT_TRUE(m.Tso == fr.Tso);
    EXPECT_TRUE(m.Te == fr.Te);
    EXPECT_TRUE(m.Tdemin == fr.Tdemin);
}

TEST(ActionMessage, constructor_test)
{
    // Default constructor
    helics::ActionMessage cmd;
    EXPECT_TRUE(cmd.action() == helics::CMD_IGNORE);
    EXPECT_EQ(cmd.source_id, parent_broker_id);
    EXPECT_TRUE(!cmd.source_handle.isValid());
    EXPECT_EQ(cmd.dest_id, parent_broker_id);
    EXPECT_TRUE(!cmd.dest_handle.isValid());
    EXPECT_EQ(cmd.counter, 0);
    EXPECT_EQ(cmd.flags, 0);
    EXPECT_EQ(cmd.sequenceID, 0U);
    EXPECT_EQ(cmd.actionTime, helics::Time::zeroVal());
    EXPECT_TRUE(cmd.payload.empty());

    // Additional info defaults
    EXPECT_EQ(cmd.Te, helics::timeZero);
    EXPECT_EQ(cmd.Tdemin, helics::timeZero);
    EXPECT_TRUE(cmd.getStringData().empty());

    // Action constructor
    helics::ActionMessage cmd2(helics::CMD_INIT);
    EXPECT_TRUE(cmd2.action() == helics::CMD_INIT);
}

TEST(ActionMessage, copy_constructor_test)
{
    helics::ActionMessage cmd(helics::CMD_INIT);
    cmd.source_id = global_federate_id{1};
    cmd.source_handle = interface_handle{2};
    cmd.dest_id = global_federate_id{3};
    cmd.dest_handle = interface_handle{4};
    cmd.flags = 0x1a2F;  // this has no significance
    cmd.actionTime = helics::Time::maxVal();
    cmd.payload = "hello world";
    cmd.sequenceID = 876;
    cmd.Te = helics::Time::maxVal();
    cmd.Tdemin = helics::Time::minVal();
    cmd.setStringData("target", "source", "original_source");

    // Check operator= override
    helics::ActionMessage cmd_copy(cmd);
    EXPECT_TRUE(cmd_copy.action() == helics::CMD_INIT);
    EXPECT_EQ(cmd_copy.source_id.baseValue(), 1);
    EXPECT_EQ(cmd_copy.source_handle.baseValue(), 2);
    EXPECT_EQ(cmd_copy.dest_id.baseValue(), 3);
    EXPECT_EQ(cmd_copy.dest_handle.baseValue(), 4);
    EXPECT_EQ(cmd_copy.flags, 0x1a2F);
    EXPECT_EQ(cmd_copy.sequenceID, 876U);
    EXPECT_EQ(cmd_copy.actionTime, helics::Time::maxVal());
    EXPECT_EQ(cmd_copy.payload, "hello world");
    EXPECT_EQ(cmd_copy.name, "hello world");  // aliased to payload

    EXPECT_EQ(cmd_copy.Te, helics::Time::maxVal());
    EXPECT_EQ(cmd_copy.Tdemin, helics::Time::minVal());
    EXPECT_EQ(cmd_copy.getString(sourceStringLoc), "source");
    EXPECT_EQ(cmd_copy.getString(targetStringLoc), "target");
    EXPECT_EQ(cmd_copy.getString(origSourceStringLoc), "original_source");
}

TEST(ActionMessage, assignment_test)
{
    helics::ActionMessage cmd(helics::CMD_INIT);
    cmd.source_id = global_federate_id(1);
    cmd.source_handle = interface_handle(2);
    cmd.dest_id = global_federate_id(3);
    cmd.dest_handle = interface_handle(4);
    cmd.sequenceID = 876U;
    setActionFlag(cmd, iteration_requested_flag);
    setActionFlag(cmd, required_flag);
    setActionFlag(cmd, error_flag);
    cmd.actionTime = helics::Time::maxVal();
    cmd.payload = "hello world";

    cmd.Te = helics::Time::maxVal();
    cmd.Tdemin = helics::Time::minVal();
    cmd.setStringData("target", "source", "original_source", "original_dest");

    // Check operator= override
    helics::ActionMessage cmd_assign = cmd;
    EXPECT_TRUE(cmd_assign.action() == helics::CMD_INIT);
    EXPECT_EQ(cmd_assign.source_id.baseValue(), 1);
    EXPECT_EQ(cmd_assign.source_handle.baseValue(), 2);
    EXPECT_EQ(cmd_assign.dest_id.baseValue(), 3);
    EXPECT_EQ(cmd_assign.dest_handle.baseValue(), 4);
    EXPECT_TRUE(checkActionFlag(cmd_assign, iteration_requested_flag));
    EXPECT_TRUE(checkActionFlag(cmd_assign, required_flag));
    EXPECT_TRUE(checkActionFlag(cmd_assign, error_flag));
    EXPECT_EQ(cmd_assign.actionTime, helics::Time::maxVal());
    EXPECT_EQ(cmd_assign.payload, "hello world");
    EXPECT_EQ(cmd_assign.name, "hello world");  // aliased to payload
    EXPECT_EQ(cmd_assign.sequenceID, 876U);

    EXPECT_EQ(cmd_assign.Te, helics::Time::maxVal());
    EXPECT_EQ(cmd_assign.Tdemin, helics::Time::minVal());
    EXPECT_EQ(cmd_assign.getString(sourceStringLoc), "source");
    EXPECT_EQ(cmd_assign.getString(targetStringLoc), "target");
    EXPECT_EQ(cmd_assign.getString(origSourceStringLoc), "original_source");
    EXPECT_EQ(cmd_assign.getString(origDestStringLoc), "original_dest");
}

TEST(ActionMessage, comparison_test)
{
    helics::ActionMessage cmd1(helics::CMD_INIT);
    cmd1.actionTime = helics::Time::minVal();

    helics::ActionMessage cmd2(helics::CMD_INIT);
    cmd2.actionTime = helics::Time::zeroVal();

    helics::ActionMessage cmd3(helics::CMD_INIT);
    cmd3.actionTime = helics::Time::maxVal();

    // Check less than comparison (not implemented yet)

    EXPECT_TRUE(cmd1 < cmd2);
    EXPECT_TRUE(cmd2 < cmd3);
    EXPECT_TRUE(cmd1 < cmd3);
    EXPECT_TRUE(!(cmd1 < cmd1));

    // Insert messages into a set, check for correct ordering when scanned
    std::set<helics::ActionMessage> q;
    q.insert(cmd2);
    q.insert(cmd3);
    q.insert(cmd1);
    helics::Time ctime = helics::Time::minVal();
    for (const auto& cmd : q) {
        EXPECT_TRUE(cmd.actionTime >= ctime);
        ctime = cmd.actionTime;
    }
}

TEST(ActionMessage, conversion_test)
{
    helics::ActionMessage cmd(helics::CMD_SEND_MESSAGE);
    cmd.source_id = global_federate_id(1);
    cmd.source_handle = interface_handle(2);
    cmd.dest_id = global_federate_id(3);
    cmd.dest_handle = interface_handle(4);
    cmd.messageID = 762354;
    setActionFlag(cmd, iteration_requested_flag);
    setActionFlag(cmd, required_flag);
    setActionFlag(cmd, error_flag);
    cmd.actionTime = 45.7;
    cmd.payload = std::string(5000, 'a');

    cmd.setStringData("target", "source as a very long string test .........", "original_source");

    auto cmdString = cmd.to_string();

    helics::ActionMessage cmd2(cmdString);
    EXPECT_TRUE(cmd.action() == cmd2.action());
    EXPECT_EQ(cmd.actionTime, cmd2.actionTime);
    EXPECT_EQ(cmd.source_id, cmd2.source_id);
    EXPECT_EQ(cmd.dest_id, cmd2.dest_id);
    EXPECT_EQ(cmd.source_handle, cmd2.source_handle);
    EXPECT_EQ(cmd.dest_handle, cmd2.dest_handle);
    EXPECT_EQ(cmd.payload, cmd2.payload);
    EXPECT_EQ(cmd.flags, cmd2.flags);
    EXPECT_EQ(cmd.messageID, 762354);
    EXPECT_TRUE(cmd.getStringData() == cmd2.getStringData());
}

TEST(ActionMessage_tests, conversion_test2)
{
    helics::ActionMessage cmd(helics::CMD_SEND_MESSAGE);
    cmd.source_id = global_federate_id{1};
    cmd.source_handle = interface_handle{2};
    cmd.dest_id = global_federate_id{3};
    cmd.dest_handle = interface_handle{4};
    setActionFlag(cmd, iteration_requested_flag);
    setActionFlag(cmd, required_flag);
    setActionFlag(cmd, error_flag);
    cmd.actionTime = 45.7;
    cmd.payload = std::string(500000, 'j');

    cmd.setStringData("target", "source as a very long string test .........", "original_source");

    auto cmdString = cmd.to_string();

    helics::ActionMessage cmd2(cmdString);
    EXPECT_TRUE(cmd.action() == cmd2.action());
    EXPECT_EQ(cmd.actionTime, cmd2.actionTime);
    EXPECT_EQ(cmd.source_id, cmd2.source_id);
    EXPECT_EQ(cmd.dest_id, cmd2.dest_id);
    EXPECT_EQ(cmd.source_handle, cmd2.source_handle);
    EXPECT_EQ(cmd.dest_handle, cmd2.dest_handle);
    EXPECT_EQ(cmd.payload, cmd2.payload);
    EXPECT_EQ(cmd.flags, cmd2.flags);
    EXPECT_TRUE(cmd.getStringData() == cmd2.getStringData());
}

TEST(ActionMessage_tests, message_message_conversion_test)
{
    helics::ActionMessage cmd(helics::CMD_SEND_MESSAGE);
    cmd.source_id = global_federate_id{1};
    cmd.source_handle = interface_handle{2};
    cmd.dest_id = global_federate_id{3};
    cmd.dest_handle = interface_handle{4};
    setActionFlag(cmd, iteration_requested_flag);
    setActionFlag(cmd, required_flag);
    setActionFlag(cmd, error_flag);
    cmd.actionTime = 45.7;
    cmd.payload = "hello world";

    cmd.setStringData("target", "source as a very long string test .........", "original_source");

    auto msg = helics::createMessageFromCommand(cmd);

    EXPECT_EQ(cmd.actionTime, msg->time);
    EXPECT_EQ(cmd.getString(sourceStringLoc), msg->source);
    EXPECT_EQ(cmd.getString(origSourceStringLoc), msg->original_source);
    EXPECT_EQ(cmd.getString(targetStringLoc), msg->dest);
    EXPECT_EQ(cmd.payload, msg->data.to_string());

    ActionMessage cmd2;
    cmd2 = std::move(msg);
    EXPECT_TRUE(cmd.action() == CMD_SEND_MESSAGE);
    EXPECT_EQ(cmd.actionTime, cmd2.actionTime);
    EXPECT_EQ(cmd.getString(0), cmd2.getString(0));
    EXPECT_EQ(cmd.getString(1), cmd2.getString(1));
    EXPECT_EQ(cmd.getString(2), cmd2.getString(2));
    EXPECT_EQ(cmd.payload, cmd.payload);
}

// check some error handling in the toByteArray function
TEST(ActionMessage_tests, check_conversions)
{
    helics::ActionMessage cmd(helics::CMD_PROTOCOL);
    cmd.messageID = 10;
    cmd.payload = "this is a payload test";

    auto cmdStr = cmd.to_string();
    auto cmdVec = cmd.to_vector();
    EXPECT_EQ(cmdStr.size(), cmdVec.size());
    EXPECT_EQ(cmdStr, std::string(cmdVec.data(), cmdVec.size()));

    auto testBuffer1 = std::make_unique<char[]>(cmdStr.size() + 20);
    auto testBuffer2 = std::make_unique<char[]>(cmdStr.size() >> 2U);  // make a too small buffer

    auto res = cmd.toByteArray(testBuffer1.get(), static_cast<int>(cmdStr.size() + 20));
    EXPECT_EQ(res, static_cast<int>(cmdStr.size()));
    // just check to make sure the same string was written
    EXPECT_EQ(cmdStr, std::string(testBuffer1.get(), res));
    // this should return -1
    res = cmd.toByteArray(testBuffer2.get(), static_cast<int>(cmdStr.size() >> 2U));
    EXPECT_EQ(res, -1);
}

// check some error handling in the toByteArray function
TEST(ActionMessage_tests, check_packetization)
{
    helics::ActionMessage cmd(helics::CMD_SEND_MESSAGE);
    cmd.source_id = global_federate_id(1);
    cmd.source_handle = interface_handle(2);
    cmd.dest_id = global_federate_id(3);
    cmd.dest_handle = interface_handle(4);
    setActionFlag(cmd, iteration_requested_flag);
    setActionFlag(cmd, required_flag);
    setActionFlag(cmd, error_flag);
    cmd.actionTime = 45.7;
    cmd.payload = "hello world";

    cmd.setStringData("target", "source as a very long string test .........", "original_source");
    auto cmdStringNormal = cmd.to_string();
    auto cmdString = cmd.packetize();
    EXPECT_GE(cmdStringNormal.size() + 6, cmdString.size());
    helics::ActionMessage cmd2;
    auto res = cmd2.depacketize(cmdString.data(), static_cast<int>(cmdString.size()));
    EXPECT_EQ(res, static_cast<int>(cmdString.size()));
    EXPECT_TRUE(cmd.action() == cmd2.action());
    EXPECT_EQ(cmd.actionTime, cmd2.actionTime);
    EXPECT_EQ(cmd.source_id, cmd2.source_id);
    EXPECT_EQ(cmd.dest_id, cmd2.dest_id);
    EXPECT_EQ(cmd.source_handle, cmd2.source_handle);
    EXPECT_EQ(cmd.dest_handle, cmd2.dest_handle);
    EXPECT_EQ(cmd.payload, cmd2.payload);
    EXPECT_EQ(cmd.flags, cmd2.flags);
    EXPECT_TRUE(cmd.getStringData() == cmd2.getStringData());
}
