/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CombinationFederate.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <thread>

int main(int argc, char* argv[])  // NOLINT
{
    helics::FederateInfo fedInfo(argc, argv);
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
    helics::BrokerApp brk(fedInfo.coreType, fedInfo.brokerInitString + " -f 2");

    auto cFed = std::make_unique<helics::CombinationFederate>("ioFed1", fedInfo);

    helics::SmallBuffer mbuf(256, 0);
    for (int ii = 0; ii < 256; ++ii) {
        mbuf[ii] = std::byte(ii);
    }
    // this line actually creates an endpoint
    auto& ept = cFed->registerEndpoint("ept");

    auto& pubid = cFed->registerPublication("pub", "double");

    auto& subid = cFed->registerSubscription("ioFed2/pub");

    cFed->enterInitializingMode();
    cFed->enterExecutingMode();
    bool passed{true};
    for (int ii = 1; ii < 10; ++ii) {
        std::string mback = std::string(" at time ") + std::to_string(ii);
        std::string message = std::string("message sent from ioFed1 to ioFed2");
        message.append(mback);
        ept.sendTo(message.data(), message.size(), "ioFed2/ept");
        ept.sendTo(mbuf, "ioFed2/ept");
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
                passed = false;
            }
        }
    }
    cFed->finalize();
    brk.waitForDisconnect();
    if (passed) {
        std::cout << "Federate1 has PASSED the test";
    } else {
        std::cout << "Federate1 has FAILED the test";
    }
    return 0;
}
