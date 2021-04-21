/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/BasicHandleInfo.hpp"
#include "helics/core/EndpointInfo.hpp"
#include "helics/core/FilterInfo.hpp"
#include "helics/core/InputInfo.hpp"

#include "gtest/gtest.h"

TEST(InfoClass_tests, basichandleinfo_test)
{
    // All default values
    helics::BasicHandleInfo defHnd;
    EXPECT_TRUE(!defHnd.handle.isValid());
    EXPECT_TRUE(!defHnd.local_fed_id.isValid());
    EXPECT_TRUE(defHnd.handleType == helics::handle_type::unknown);
    EXPECT_EQ(defHnd.flags, 0);
    EXPECT_TRUE(defHnd.key.empty());
    EXPECT_TRUE(defHnd.type.empty());
    EXPECT_TRUE(defHnd.units.empty());

    // Constructor with last parameter default value
    helics::BasicHandleInfo hnd1(helics::global_federate_id(15),
                                 helics::interface_handle(10),
                                 helics::handle_type::endpoint,
                                 "key",
                                 "type",
                                 "units");
    EXPECT_EQ(hnd1.getInterfaceHandle().baseValue(), 10);
    EXPECT_EQ(hnd1.getFederateId().baseValue(), 15);
    EXPECT_TRUE(!hnd1.local_fed_id.isValid());
    EXPECT_TRUE(hnd1.handleType == helics::handle_type::endpoint);
    EXPECT_EQ(hnd1.flags, 0);
    EXPECT_EQ(hnd1.key, "key");
    EXPECT_EQ(hnd1.type, "type");
    EXPECT_EQ(hnd1.units, "units");

    // Constructor overriding last parameter default value
    helics::BasicHandleInfo hnd2(helics::global_federate_id(1500),
                                 helics::interface_handle(100),
                                 helics::handle_type::endpoint,
                                 "key",
                                 "type",
                                 "units");
    EXPECT_EQ(hnd2.getInterfaceHandle().baseValue(), 100);
    EXPECT_EQ(hnd2.getFederateId().baseValue(), 1500);
    EXPECT_TRUE(!hnd2.local_fed_id.isValid());
    EXPECT_TRUE(hnd2.handleType == helics::handle_type::endpoint);
    EXPECT_EQ(hnd1.flags, 0);
    EXPECT_EQ(hnd2.key, "key");
    EXPECT_EQ(hnd2.type, "type");
    EXPECT_EQ(hnd2.units, "units");

    // Test handles created with HANDLE_FILTER

    // Source filter handle
    // destFilter should be false, and target should be equal to what was passed in for units
    helics::BasicHandleInfo srcFiltHnd(helics::global_federate_id(2),
                                       helics::interface_handle(1),
                                       helics::handle_type::filter,
                                       "key",
                                       "type_in",
                                       "type_out");
    EXPECT_EQ(srcFiltHnd.getInterfaceHandle().baseValue(), 1);
    EXPECT_EQ(srcFiltHnd.getFederateId().baseValue(), 2);
    EXPECT_TRUE(!srcFiltHnd.local_fed_id.isValid());
    EXPECT_TRUE(srcFiltHnd.handleType == helics::handle_type::filter);
    EXPECT_EQ(hnd1.flags, 0);
    EXPECT_EQ(srcFiltHnd.key, "key");
    EXPECT_EQ(srcFiltHnd.type_in, "type_in");
    EXPECT_EQ(srcFiltHnd.type_out, "type_out");

    // Destination filter handle
    // destFilter should be true, and target should be equal to what was passed in for units
    helics::BasicHandleInfo dstFiltHnd(helics::global_federate_id(3),
                                       helics::interface_handle(7),
                                       helics::handle_type::filter,
                                       "key",
                                       "type_in",
                                       "type_out");
    EXPECT_EQ(dstFiltHnd.getInterfaceHandle().baseValue(), 7);
    EXPECT_EQ(dstFiltHnd.getFederateId().baseValue(), 3);
    EXPECT_TRUE(!dstFiltHnd.local_fed_id.isValid());
    EXPECT_TRUE(dstFiltHnd.handleType == helics::handle_type::filter);
    EXPECT_EQ(dstFiltHnd.key, "key");
    EXPECT_EQ(dstFiltHnd.type_in, "type_in");
    EXPECT_EQ(dstFiltHnd.type_out, "type_out");
}

TEST(InfoClass_tests, endpointinfo_test)
{
    // Mostly testing ordering of message sorting and maxTime function arguments

    helics::Time maxT = helics::Time::maxVal();
    helics::Time minT = helics::Time::minVal();
    helics::Time zeroT = helics::Time::zeroVal();
    // helics::Time eps = helics::Time::epsilon ();

    auto msg_time_max = std::make_unique<helics::Message>();
    msg_time_max->data = "maxT";
    msg_time_max->original_source = "aFed";
    msg_time_max->time = maxT;

    auto msg_time_min = std::make_unique<helics::Message>();
    msg_time_min->data = "minT";
    msg_time_min->original_source = "aFed";
    msg_time_min->time = minT;

    auto msg_time_zero = std::make_unique<helics::Message>();
    msg_time_zero->data = "zeroT";
    msg_time_zero->original_source = "aFed";
    msg_time_zero->time = zeroT;

    auto msg_time_one_a = std::make_unique<helics::Message>();
    msg_time_one_a->data = "oneAT";
    msg_time_one_a->original_source = "aFed";
    msg_time_one_a->time = helics::Time(1);

    auto msg_time_one_b = std::make_unique<helics::Message>();
    msg_time_one_b->data = "oneBT";
    msg_time_one_b->original_source = "bFed";
    msg_time_one_b->time = helics::Time(1);

    helics::EndpointInfo endPI({helics::global_federate_id(5), helics::interface_handle(13)},
                               "name",
                               "type");
    EXPECT_EQ(endPI.id.handle.baseValue(), 13);
    EXPECT_EQ(endPI.id.fed_id.baseValue(), 5);
    EXPECT_EQ(endPI.key, "name");
    EXPECT_EQ(endPI.type, "type");

    // Check proper return values for empty queue
    // size 0 for any time given
    // first message time is the max possible
    // nullptr for getting a message
    EXPECT_EQ(endPI.queueSize(minT), 0);
    EXPECT_EQ(endPI.queueSize(maxT), 0);
    EXPECT_EQ(endPI.firstMessageTime(), maxT);
    EXPECT_TRUE(endPI.getMessage(minT) == nullptr);
    EXPECT_TRUE(endPI.getMessage(maxT) == nullptr);

    // Add a message at the max time possible
    endPI.addMessage(std::move(msg_time_max));
    EXPECT_EQ(endPI.queueSize(minT), 0);
    EXPECT_EQ(endPI.queueSize(maxT), 1);
    EXPECT_EQ(endPI.firstMessageTime(), maxT);

    // Add a message at time zero (check if maxTime parameter is working queueSize())
    endPI.addMessage(std::move(msg_time_zero));
    EXPECT_EQ(endPI.queueSize(minT), 0);
    EXPECT_EQ(endPI.queueSize(zeroT), 1);
    EXPECT_EQ(endPI.queueSize(maxT), 2);
    EXPECT_EQ(endPI.firstMessageTime(), zeroT);

    // Add a message at the min time possible
    endPI.addMessage(std::move(msg_time_min));
    EXPECT_EQ(endPI.queueSize(minT), 1);
    EXPECT_EQ(endPI.queueSize(zeroT), 2);
    EXPECT_EQ(endPI.queueSize(1), 2);
    EXPECT_EQ(endPI.queueSize(maxT), 3);
    EXPECT_EQ(endPI.firstMessageTime(), minT);

    // Add a message at a time somewhere in between the others
    endPI.addMessage(std::move(msg_time_one_b));
    EXPECT_EQ(endPI.queueSize(minT), 1);
    EXPECT_EQ(endPI.queueSize(zeroT), 2);
    EXPECT_EQ(endPI.queueSize(1), 3);
    EXPECT_EQ(endPI.queueSize(maxT), 4);
    EXPECT_EQ(endPI.firstMessageTime(), minT);

    endPI.updateTimeInclusive(minT);
    // Test maxTime parameter for getMessage(), and proper dequeuing
    auto msg = endPI.getMessage(minT);
    EXPECT_EQ(msg->data.to_string(), "minT");
    EXPECT_EQ(endPI.queueSize(minT), 0);
    EXPECT_EQ(endPI.queueSize(maxT), 3);
    EXPECT_TRUE(endPI.getMessage(minT) == nullptr);

    // Message at time 0 should now be the first
    EXPECT_EQ(endPI.firstMessageTime(), zeroT);
    endPI.updateTimeInclusive(zeroT);
    msg = endPI.getMessage(zeroT);
    EXPECT_EQ(msg->data.to_string(), "zeroT");
    EXPECT_EQ(endPI.queueSize(maxT), 2);
    EXPECT_TRUE(endPI.getMessage(zeroT) == nullptr);

    // Now message at time 1 (bFed) should be the first
    EXPECT_EQ(endPI.firstMessageTime(), helics::Time(1));
    endPI.updateTimeInclusive(1);
    EXPECT_EQ(endPI.availableMessages(), 1);

    // Insert another message at time 1 (aFed) to test ordering by original source name
    endPI.addMessage(std::move(msg_time_one_a));
    endPI.updateTimeInclusive(1);
    EXPECT_EQ(endPI.firstMessageTime(), helics::Time(1));
    EXPECT_EQ(endPI.availableMessages(), 2);
    endPI.updateTimeInclusive(1);
    msg = endPI.getMessage(1);
    EXPECT_EQ(msg->data.to_string(), "oneAT");
    EXPECT_EQ(endPI.queueSize(1), 1);
    msg = endPI.getMessage(1);
    EXPECT_EQ(msg->data.to_string(), "oneBT");
    EXPECT_EQ(endPI.queueSize(1), 0);
    EXPECT_TRUE(endPI.getMessage(1) == nullptr);

    // Recreate messages A and B at time 1
    msg_time_one_a = std::make_unique<helics::Message>();
    msg_time_one_a->data = "oneAT";
    msg_time_one_a->original_source = "aFed";
    msg_time_one_a->time = helics::Time(1);

    msg_time_one_b = std::make_unique<helics::Message>();
    msg_time_one_b->data = "oneBT";
    msg_time_one_b->original_source = "bFed";
    msg_time_one_b->time = helics::Time(1);

    // Perform the same source name federate test, but reverse order of add messages
    endPI.addMessage(std::move(msg_time_one_a));
    endPI.addMessage(std::move(msg_time_one_b));
    EXPECT_EQ(endPI.queueSize(1), 2);
    endPI.updateTimeInclusive(1);
    msg = endPI.getMessage(1);
    EXPECT_EQ(msg->data.to_string(), "oneAT");
    EXPECT_EQ(endPI.queueSize(1), 1);
    msg = endPI.getMessage(1);
    EXPECT_EQ(msg->data.to_string(), "oneBT");
    EXPECT_EQ(endPI.queueSize(1), 0);
    endPI.updateTimeInclusive(maxT);
    // Test removing all elements from queue
    msg = endPI.getMessage(maxT);
    ASSERT_TRUE(msg);
    EXPECT_EQ(msg->data.to_string(), "maxT");
    EXPECT_EQ(endPI.queueSize(maxT), 0);
    EXPECT_TRUE(endPI.getMessage(maxT) == nullptr);
}

TEST(InfoClass_tests, filterinfo_test)
{
    // Mostly testing ordering of message sorting and maxTime function arguments

    helics::Time maxT = helics::Time::maxVal();
    helics::Time minT = helics::Time::minVal();
    helics::Time zeroT = helics::Time::zeroVal();

    auto msg_time_max = std::make_unique<helics::Message>();
    msg_time_max->data = "maxT";
    msg_time_max->original_source = "aFed";
    msg_time_max->time = maxT;

    auto msg_time_min = std::make_unique<helics::Message>();
    msg_time_min->data = "minT";
    msg_time_min->original_source = "aFed";
    msg_time_min->time = minT;

    auto msg_time_zero = std::make_unique<helics::Message>();
    msg_time_zero->data = "zeroT";
    msg_time_zero->original_source = "aFed";
    msg_time_zero->time = zeroT;

    auto msg_time_one_a = std::make_unique<helics::Message>();
    msg_time_one_a->data = "oneAT";
    msg_time_one_a->original_source = "aFed";
    msg_time_one_a->time = helics::Time(1);

    auto msg_time_one_b = std::make_unique<helics::Message>();
    msg_time_one_b->data = "oneBT";
    msg_time_one_b->original_source = "bFed";
    msg_time_one_b->time = helics::Time(1);

    helics::FilterInfo filtI(helics::global_broker_id(5),
                             helics::interface_handle(13),
                             "name",
                             "type_in",
                             "type_out",
                             true);
    EXPECT_EQ(filtI.handle.baseValue(), 13);
    EXPECT_EQ(filtI.core_id.baseValue(), 5);
    EXPECT_EQ(filtI.key, "name");
    EXPECT_EQ(filtI.inputType, "type_in");
    EXPECT_EQ(filtI.outputType, "type_out");
    EXPECT_EQ(filtI.dest_filter, true);
}

TEST(InfoClass_tests, inputinfo_test)
{
    // SubscriptionInfo is still a WPI, nothing moves data from the queue to current_data

    std::shared_ptr<const helics::data_block> ret_data;

    helics::InputInfo subI(helics::global_handle(helics::global_federate_id(5),
                                                 helics::interface_handle(13)),
                           "key",
                           "type",
                           "units");
    EXPECT_EQ(subI.id.handle.baseValue(), 13);
    EXPECT_EQ(subI.id.fed_id.baseValue(), 5);
    EXPECT_EQ(subI.key, "key");
    EXPECT_EQ(subI.type, "type");
    EXPECT_EQ(subI.units, "units");
    EXPECT_EQ(subI.required, false);

    helics::global_handle testHandle(helics::global_federate_id(5), helics::interface_handle(45));
    subI.addSource(testHandle, "", "double", std::string());
    // No data available, shouldn't get a data_block back
    ret_data = subI.getData(0);
    EXPECT_TRUE(!ret_data);

    auto hello_data = std::make_shared<helics::data_block>("hello world");
    subI.addData(testHandle, helics::timeZero, 0, hello_data);
    subI.updateTimeInclusive(helics::timeZero);
    ret_data = subI.getData(0);

    EXPECT_EQ(ret_data->size(), 11U);
    EXPECT_EQ(ret_data->to_string(), hello_data->to_string());

    auto time_one_data = std::make_shared<helics::data_block>("time one");
    auto time_one_repeat_data = std::make_shared<helics::data_block>("time one repeat");
    subI.addData(testHandle, 1, 0, time_one_data);
    subI.addData(testHandle, 1, 0, time_one_repeat_data);

    subI.updateTimeInclusive(1.0);
    ret_data = subI.getData(0);
    EXPECT_EQ(ret_data->to_string(), "time one repeat");
    subI.addData(testHandle, 2, 0, time_one_data);
    subI.addData(testHandle, 2, 1, time_one_repeat_data);

    subI.updateTimeNextIteration(2.0);
    ret_data = subI.getData(0);
    EXPECT_EQ(ret_data->to_string(), "time one");
}
