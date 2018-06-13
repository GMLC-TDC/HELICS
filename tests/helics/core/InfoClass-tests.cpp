/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/BasicHandleInfo.hpp"
#include "helics/core/EndpointInfo.hpp"
#include "helics/core/FilterInfo.hpp"
#include "helics/core/SubscriptionInfo.hpp"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (InfoClass_tests, *utf::label("ci") *utf::label("daily") *utf::label("release"))

BOOST_AUTO_TEST_CASE (basichandleinfo_test)
{
    // All default values
    helics::BasicHandleInfo defHnd;
    BOOST_CHECK_EQUAL (defHnd.handle, helics::invalid_handle);
    BOOST_CHECK_EQUAL (defHnd.fed_id, helics::invalid_fed_id);
    BOOST_CHECK_EQUAL (defHnd.local_fed_id, helics::invalid_fed_id);
    BOOST_CHECK (defHnd.handle_type==helics::handle_type_t::unknown);
    BOOST_CHECK_EQUAL (defHnd.flags, 0);
    BOOST_CHECK (defHnd.key.empty ());
    BOOST_CHECK (defHnd.type.empty ());
    BOOST_CHECK (defHnd.units.empty ());
    BOOST_CHECK (defHnd.target.empty ());

    // Constructor with last parameter default value
    helics::BasicHandleInfo hnd1 (10, 15, helics::handle_type_t::endpoint, "key", "type", "units");
    BOOST_CHECK_EQUAL (hnd1.handle, 10);
    BOOST_CHECK_EQUAL (hnd1.fed_id, 15);
    BOOST_CHECK_EQUAL (hnd1.local_fed_id, helics::invalid_fed_id);
    BOOST_CHECK(hnd1.handle_type==helics::handle_type_t::endpoint);
    BOOST_CHECK_EQUAL(hnd1.flags, 0);
    BOOST_CHECK_EQUAL(hnd1.key,"key");
    BOOST_CHECK_EQUAL(hnd1.type,"type");
    BOOST_CHECK_EQUAL(hnd1.units,"units");
    BOOST_CHECK (hnd1.target.empty ());

    // Constructor overriding last parameter default value
    helics::BasicHandleInfo hnd2 (100, 1500, helics::handle_type_t::endpoint, "key", "type", "units");
    BOOST_CHECK_EQUAL (hnd2.handle, 100);
    BOOST_CHECK_EQUAL (hnd2.fed_id, 1500);
    BOOST_CHECK_EQUAL (hnd2.local_fed_id, helics::invalid_fed_id);
    BOOST_CHECK (hnd2.handle_type==helics::handle_type_t::endpoint);
    BOOST_CHECK_EQUAL(hnd1.flags, 0);
    BOOST_CHECK_EQUAL(hnd2.key,"key");
    BOOST_CHECK_EQUAL(hnd2.type,"type");
    BOOST_CHECK_EQUAL(hnd2.units,"units");
    BOOST_CHECK(hnd2.target.empty ());

    // Test handles created with HANDLE_FILTER

    // Source filter handle
    // destFilter should be false, and target should be equal to what was passed in for units
    helics::BasicHandleInfo srcFiltHnd (1, 2, helics::handle_type_t::source_filter, "key", "target", "type_in",
                                        "type_out");
    BOOST_CHECK_EQUAL (srcFiltHnd.handle, 1);
    BOOST_CHECK_EQUAL (srcFiltHnd.fed_id, 2);
    BOOST_CHECK_EQUAL (srcFiltHnd.local_fed_id, helics::invalid_fed_id);
    BOOST_CHECK(srcFiltHnd.handle_type==helics::handle_type_t::source_filter);
    BOOST_CHECK_EQUAL(hnd1.flags, 0);
    BOOST_CHECK_EQUAL(srcFiltHnd.key,"key");
    BOOST_CHECK_EQUAL(srcFiltHnd.type_in,"type_in");
    BOOST_CHECK_EQUAL(srcFiltHnd.type_out,"type_out");
    BOOST_CHECK_EQUAL(srcFiltHnd.target,"target");

    // Destination filter handle
    // destFilter should be true, and target should be equal to what was passed in for units
    helics::BasicHandleInfo dstFiltHnd (7, 3, helics::handle_type_t::destination_filter, "key", "target", "type_in", "type_out");
    BOOST_CHECK_EQUAL (dstFiltHnd.handle, 7);
    BOOST_CHECK_EQUAL (dstFiltHnd.fed_id, 3);
    BOOST_CHECK_EQUAL(dstFiltHnd.local_fed_id,helics::invalid_fed_id);
    BOOST_CHECK(dstFiltHnd.handle_type==helics::handle_type_t::destination_filter);
    BOOST_CHECK_EQUAL (dstFiltHnd.key,"key");
    BOOST_CHECK_EQUAL (dstFiltHnd.type_in,"type_in");
    BOOST_CHECK_EQUAL (dstFiltHnd.type_out,"type_out");
    BOOST_CHECK_EQUAL (dstFiltHnd.target,"target");
}

BOOST_AUTO_TEST_CASE (endpointinfo_test)
{
    // Mostly testing ordering of message sorting and maxTime function arguments

    helics::Time maxT = helics::Time::maxVal ();
    helics::Time minT = helics::Time::minVal ();
    helics::Time zeroT = helics::Time::zeroVal ();
    // helics::Time eps = helics::Time::epsilon ();

    auto msg_time_max = std::make_unique<helics::Message> ();
    msg_time_max->data = "maxT";
    msg_time_max->original_source = "aFed";
    msg_time_max->time = maxT;

    auto msg_time_min = std::make_unique<helics::Message> ();
    msg_time_min->data = "minT";
    msg_time_min->original_source = "aFed";
    msg_time_min->time = minT;

    auto msg_time_zero = std::make_unique<helics::Message> ();
    msg_time_zero->data = "zeroT";
    msg_time_zero->original_source = "aFed";
    msg_time_zero->time = zeroT;

    auto msg_time_one_a = std::make_unique<helics::Message> ();
    msg_time_one_a->data = "oneAT";
    msg_time_one_a->original_source = "aFed";
    msg_time_one_a->time = helics::Time (1);

    auto msg_time_one_b = std::make_unique<helics::Message> ();
    msg_time_one_b->data = "oneBT";
    msg_time_one_b->original_source = "bFed";
    msg_time_one_b->time = helics::Time (1);

    helics::EndpointInfo endPI (13, 5, "name", "type");
    BOOST_CHECK_EQUAL (endPI.id, 13);
    BOOST_CHECK_EQUAL (endPI.fed_id, 5);
    BOOST_CHECK (endPI.key.compare ("name") == 0);
    BOOST_CHECK (endPI.type.compare ("type") == 0);

    // Check proper return values for empty queue
    // size 0 for any time given
    // first message time is the max possible
    // nullptr for getting a message
    BOOST_CHECK_EQUAL (endPI.queueSize (minT), 0);
    BOOST_CHECK_EQUAL (endPI.queueSize (maxT), 0);
    BOOST_CHECK_EQUAL (endPI.firstMessageTime (), maxT);
    BOOST_CHECK (endPI.getMessage (minT) == nullptr);
    BOOST_CHECK (endPI.getMessage (maxT) == nullptr);

    // Add a message at the max time possible
    endPI.addMessage (std::move (msg_time_max));
    BOOST_CHECK_EQUAL (endPI.queueSize (minT), 0);
    BOOST_CHECK_EQUAL (endPI.queueSize (maxT), 1);
    BOOST_CHECK_EQUAL (endPI.firstMessageTime (), maxT);

    // Add a message at time zero (check if maxTime parameter is working queueSize())
    endPI.addMessage (std::move (msg_time_zero));
    BOOST_CHECK_EQUAL (endPI.queueSize (minT), 0);
    BOOST_CHECK_EQUAL (endPI.queueSize (zeroT), 1);
    BOOST_CHECK_EQUAL (endPI.queueSize (maxT), 2);
    BOOST_CHECK_EQUAL (endPI.firstMessageTime (), zeroT);

    // Add a message at the min time possible
    endPI.addMessage (std::move (msg_time_min));
    BOOST_CHECK_EQUAL (endPI.queueSize (minT), 1);
    BOOST_CHECK_EQUAL (endPI.queueSize (zeroT), 2);
    BOOST_CHECK_EQUAL (endPI.queueSize (1), 2);
    BOOST_CHECK_EQUAL (endPI.queueSize (maxT), 3);
    BOOST_CHECK_EQUAL (endPI.firstMessageTime (), minT);

    // Add a message at a time somewhere in between the others
    endPI.addMessage (std::move (msg_time_one_b));
    BOOST_CHECK_EQUAL (endPI.queueSize (minT), 1);
    BOOST_CHECK_EQUAL (endPI.queueSize (zeroT), 2);
    BOOST_CHECK_EQUAL (endPI.queueSize (1), 3);
    BOOST_CHECK_EQUAL (endPI.queueSize (maxT), 4);
    BOOST_CHECK_EQUAL (endPI.firstMessageTime (), minT);

    // Test maxTime parameter for getMessage(), and proper dequeuing
    auto msg = endPI.getMessage (minT);
    BOOST_CHECK_EQUAL (msg->data.to_string (), "minT");
    BOOST_CHECK_EQUAL (endPI.queueSize (minT), 0);
    BOOST_CHECK_EQUAL (endPI.queueSize (maxT), 3);
    BOOST_CHECK (endPI.getMessage (minT) == nullptr);

    // Message at time 0 should now be the first
    BOOST_CHECK_EQUAL (endPI.firstMessageTime (), zeroT);
    msg = endPI.getMessage (zeroT);
    BOOST_CHECK_EQUAL (msg->data.to_string (), "zeroT");
    BOOST_CHECK_EQUAL (endPI.queueSize (maxT), 2);
    BOOST_CHECK (endPI.getMessage (zeroT) == nullptr);

    // Now message at time 1 (bFed) should be the first
    BOOST_CHECK_EQUAL (endPI.firstMessageTime (), helics::Time (1));
    BOOST_CHECK_EQUAL (endPI.queueSize (1), 1);

    // Insert another message at time 1 (aFed) to test ordering by original source name
    endPI.addMessage (std::move (msg_time_one_a));
    BOOST_CHECK_EQUAL (endPI.firstMessageTime (), helics::Time (1));
    BOOST_CHECK_EQUAL (endPI.queueSize (1), 2);
    msg = endPI.getMessage (1);
    BOOST_CHECK_EQUAL (msg->data.to_string (), "oneAT");
    BOOST_CHECK_EQUAL (endPI.queueSize (1), 1);
    msg = endPI.getMessage (1);
    BOOST_CHECK_EQUAL (msg->data.to_string (), "oneBT");
    BOOST_CHECK_EQUAL (endPI.queueSize (1), 0);
    BOOST_CHECK (endPI.getMessage (1) == nullptr);

    // Recreate messages A and B at time 1
    msg_time_one_a = std::make_unique<helics::Message> ();
    msg_time_one_a->data = "oneAT";
    msg_time_one_a->original_source = "aFed";
    msg_time_one_a->time = helics::Time (1);

    msg_time_one_b = std::make_unique<helics::Message> ();
    msg_time_one_b->data = "oneBT";
    msg_time_one_b->original_source = "bFed";
    msg_time_one_b->time = helics::Time (1);

    // Perform the same source name federate test, but reverse order of add messages
    endPI.addMessage (std::move (msg_time_one_a));
    endPI.addMessage (std::move (msg_time_one_b));
    BOOST_CHECK_EQUAL (endPI.queueSize (1), 2);
    msg = endPI.getMessage (1);
    BOOST_CHECK_EQUAL (msg->data.to_string (), "oneAT");
    BOOST_CHECK_EQUAL (endPI.queueSize (1), 1);
    msg = endPI.getMessage (1);
    BOOST_CHECK_EQUAL (msg->data.to_string (), "oneBT");
    BOOST_CHECK_EQUAL (endPI.queueSize (1), 0);

    // Test removing all elements from queue
    msg = endPI.getMessage (maxT);
    BOOST_CHECK_EQUAL (msg->data.to_string (), "maxT");
    BOOST_CHECK_EQUAL (endPI.queueSize (maxT), 0);
    BOOST_CHECK (endPI.getMessage (maxT) == nullptr);
}

BOOST_AUTO_TEST_CASE (filterinfo_test)
{
    // Mostly testing ordering of message sorting and maxTime function arguments

    helics::Time maxT = helics::Time::maxVal ();
    helics::Time minT = helics::Time::minVal ();
    helics::Time zeroT = helics::Time::zeroVal ();

    auto msg_time_max = std::make_unique<helics::Message> ();
    msg_time_max->data = "maxT";
    msg_time_max->original_source = "aFed";
    msg_time_max->time = maxT;

    auto msg_time_min = std::make_unique<helics::Message> ();
    msg_time_min->data = "minT";
    msg_time_min->original_source = "aFed";
    msg_time_min->time = minT;

    auto msg_time_zero = std::make_unique<helics::Message> ();
    msg_time_zero->data = "zeroT";
    msg_time_zero->original_source = "aFed";
    msg_time_zero->time = zeroT;

    auto msg_time_one_a = std::make_unique<helics::Message> ();
    msg_time_one_a->data = "oneAT";
    msg_time_one_a->original_source = "aFed";
    msg_time_one_a->time = helics::Time (1);

    auto msg_time_one_b = std::make_unique<helics::Message> ();
    msg_time_one_b->data = "oneBT";
    msg_time_one_b->original_source = "bFed";
    msg_time_one_b->time = helics::Time (1);

    helics::FilterInfo filtI (5, 13, "name", "target", "type_in", "type_out", true);
    BOOST_CHECK_EQUAL (filtI.handle, 13);
    BOOST_CHECK_EQUAL (filtI.fed_id, 5);
    BOOST_CHECK_EQUAL(filtI.key,"name");
    BOOST_CHECK_EQUAL(filtI.inputType,"type_in");
    BOOST_CHECK_EQUAL(filtI.outputType,"type_out");
    BOOST_CHECK_EQUAL(filtI.filterTarget,"target");
    BOOST_CHECK_EQUAL (filtI.dest_filter, true);
}

BOOST_AUTO_TEST_CASE (subscriptioninfo_test)
{
    // SubscriptionInfo is still a WPI, nothing moves data from the queue to current_data

    std::shared_ptr<const helics::data_block> ret_data;

    helics::SubscriptionInfo subI (13, 5, "key", "type", "units", true);
    BOOST_CHECK_EQUAL (subI.id, 13);
    BOOST_CHECK_EQUAL (subI.fed_id, 5);
    BOOST_CHECK_EQUAL(subI.key,"key");
    BOOST_CHECK_EQUAL(subI.type,"type");
    BOOST_CHECK_EQUAL(subI.units,"units");
    BOOST_CHECK_EQUAL (subI.required, true);

    // No data available, shouldn't get a data_block back
    ret_data = subI.getData ();
    BOOST_CHECK (!ret_data);

    // Expect this to fail
    auto hello_data = std::make_shared<helics::data_block> ("hello world");
    subI.addData (helics::timeZero, 0, hello_data);
    subI.updateTimeInclusive (helics::timeZero);
    ret_data = subI.getData ();

    // When movement of data is updated (see above), this can be uncommented and updated
    BOOST_CHECK_EQUAL (ret_data->size (), 11);
    BOOST_CHECK_EQUAL(ret_data->to_string(), hello_data->to_string());

    auto time_one_data = std::make_shared<helics::data_block> ("time one");
    auto time_one_repeat_data = std::make_shared<helics::data_block> ("time one repeat");
    subI.addData (1, 0, time_one_data);
    subI.addData (1, 0, time_one_repeat_data);

    subI.updateTimeInclusive (1.0);
    ret_data = subI.getData ();
    BOOST_CHECK_EQUAL(ret_data->to_string (),"time one repeat");
    subI.addData (2, 0, time_one_data);
    subI.addData (2, 1, time_one_repeat_data);

    subI.updateTimeNextIteration (2.0);
    ret_data = subI.getData ();
    BOOST_CHECK (ret_data->to_string () == "time one");
}

BOOST_AUTO_TEST_SUITE_END ()
