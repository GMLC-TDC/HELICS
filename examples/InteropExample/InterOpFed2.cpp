/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    helics::FederateInfo fi(argc, argv);
    fi.setProperty(helics_property_time_period, 1.0);

    auto cFed = std::make_unique<helics::CombinationFederate>("ioFed2", fi);
    auto name = cFed->getName();

    helics::data_block mbuf(256, 0);
    for (int ii = 0; ii < 256; ++ii) {
        mbuf[ii] = char(ii);
    }
    // this line actually creates an endpoint
    auto& ept = cFed->registerEndpoint("ept");

    auto& pubid = cFed->registerPublication("pub", "double");

    auto& subid = cFed->registerSubscription("ioFed1/pub");

    cFed->enterInitializingMode();
    cFed->enterExecutingMode();
    bool passed{true};
    for (int i = 1; i < 10; ++i) {
        std::string mback = std::string(" at time ") + std::to_string(i);
        std::string message = std::string("message sent from ioFed2 to ioFed1");
        message.append(mback);
        ept.send("ioFed1/ept", message.data(), message.size());
        ept.send("ioFed1/ept", mbuf);
        pubid.publish(i);

        auto newTime = cFed->requestTime(i);

        if (cFed->isUpdated(subid)) {
            auto val = subid.getValue<int>();
            if (val != i) {
                passed = false;
            }
        } else {
            passed = false;
        }
        if (ept.pendingMessages() != 2) {
            passed = false;
        } else {
            auto m1 = ept.getMessage();
            if (m1->data.to_string().find(mback) == std::string::npos) {
                passed = false;
            }
            auto m2 = ept.getMessage();
            if (mbuf != m2->data) {
                auto s1 = std::string(mbuf.to_string());
                auto s2 = std::string(m2->to_string());
                passed = false;
            }
        }
    }
    cFed->finalize();

    if (passed) {
        std::cout << "Federate 2 has PASSED the test";
    } else {
        std::cout << "Federate 2 has FAILED the test";
    }
    return 0;
}
