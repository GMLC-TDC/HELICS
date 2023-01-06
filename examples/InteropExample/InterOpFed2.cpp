/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/CombinationFederate.hpp"

#include <iostream>
#include <thread>

int main(int argc, char* argv[])  // NOLINT
{
    helics::FederateInfo fi(argc, argv);
    fi.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

    auto cFed = std::make_unique<helics::CombinationFederate>("ioFed2", fi);

    helics::SmallBuffer mbuf(256, 0);
    for (int ii = 0; ii < 256; ++ii) {
        mbuf[ii] = std::byte(ii);
    }
    // this line actually creates an endpoint
    auto& ept = cFed->registerEndpoint("ept");

    auto& pubid = cFed->registerPublication("pub", "double");

    auto& subid = cFed->registerSubscription("ioFed1/pub");

    cFed->enterInitializingMode();
    cFed->enterExecutingMode();
    bool passed{true};
    for (int ii = 1; ii < 10; ++ii) {
        std::string mback = std::string(" at time ") + std::to_string(ii);
        std::string message = std::string("message sent from ioFed2 to ioFed1");
        message.append(mback);
        ept.sendTo(message.data(), message.size(), "ioFed1/ept");
        ept.sendTo(mbuf, "ioFed1/ept");
        pubid.publish(ii);

        cFed->requestTime(ii);

        if (cFed->isUpdated(subid)) {
            auto val = subid.getValue<int>();
            if (val != ii) {
                passed = false;
            }
        } else {
            passed = false;
        }
        if (ept.pendingMessageCount() != 2) {
            passed = false;
        } else {
            auto m1 = ept.getMessage();
            if (m1->data.to_string().find(mback) == std::string_view::npos) {
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
