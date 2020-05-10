/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Inputs.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/Core.hpp"

#include "gtest/gtest.h"
#include <future>
#include <iostream>

using namespace std::literals::chrono_literals;

class FedTest {
  public:
    helics::Time deltaTime = helics::Time(10, time_units::ns);  // sampling rate
    helics::Time finalTime = helics::Time(100, time_units::ns);  // final time
  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::Publication pub;
    helics::Input sub;
    std::string fed_name;

    int pub_index = 0;
    int sub_index = 0;
    bool initialized = false;
    int counter = 0;

  public:
    FedTest() = default;

    void run()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        vFed->enterInitializingMode();
        vFed->enterExecutingMode();
        mainLoop();
    };
    void initialize(const std::string& coreName, int p_index, int s_index)
    {
        fed_name = "fedtest_" + std::to_string(p_index);
        pub_index = p_index;
        sub_index = s_index;
        helics::FederateInfo fi;
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate>(fed_name, fi);
        pub = vFed->registerPublicationIndexed<std::string>("fedrx", pub_index);
        sub = vFed->registerSubscriptionIndexed("fedrx", pub_index);
        initialized = true;
    }

    void mainLoop()
    {
        auto nextTime = deltaTime;
        std::string txstring(fed_name);

        while (nextTime < finalTime) {
            nextTime = vFed->requestTime(nextTime + deltaTime);
            vFed->publish(pub, txstring);

            if (vFed->isUpdated(sub)) {
                if (vFed->isUpdated(sub)) {
                    // get the latest value for the subscription
                    auto& nstring = vFed->getString(sub);
                    if (nstring != txstring) {
                        std::cout << "incorrect string\n";
                        break;
                    }
                    counter++;
                }
            }
        }
        ASSERT_EQ(counter, 80);
        vFed->finalize();
    }
};

TEST(ZMQSSCore_tests, zmqSSMultiCoreInitialization_test)
{
    int feds = 20;
    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ_SS,
                                                "ZMQ_SS_broker",
                                                std::string("-f ") + std::to_string(feds));
    std::vector<std::shared_ptr<helics::Core>> cores(feds);
    std::vector<FedTest> leafs(feds);
    SCOPED_TRACE("created broker");
    for (int ii = 0; ii < feds; ++ii) {
        std::string configureString = "-f 1 --name=core" + std::to_string(ii);
        cores[ii] = helics::CoreFactory::create(helics::core_type::ZMQ_SS, configureString);
        cores[ii]->connect();
        int s_index = ii + 1;
        if (ii == feds - 1) {
            s_index = 0;
        }
        leafs[ii].initialize(cores[ii]->getIdentifier(), ii, s_index);
    }
    SCOPED_TRACE("initialized");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::vector<std::thread> threads(feds);
    for (int ii = 0; ii < feds; ++ii) {
        threads[ii] = std::thread([](FedTest& leaf) { leaf.run(); }, std::ref(leafs[ii]));
    }
    SCOPED_TRACE("started threads");
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::this_thread::yield();
    for (auto& thrd : threads) {
        thrd.join();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    broker->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}
