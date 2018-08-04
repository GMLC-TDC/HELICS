/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/BrokerFactory.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

namespace bdata = boost::unit_test::data;


class RingTransmit
{
public:
    helics::Time deltaTime = helics::Time(10, timeUnits::ns);  // sampling rate
    helics::Time finalTime = helics::Time(100000, timeUnits::ns);  // final time
private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::publication_id_t pub;
    helics::input_id_t sub;

    int index_ = 0;
    int maxIndex_ = 0;
    bool initialized = false;

public:
    RingTransmit() = default;

    void run()
    {
        if (!initialized)
        {
            throw("must initialize first");
        }
        vFed->enterInitializationState();
        vFed->enterExecutionState();
        mainLoop();
    };
    void initialize(const std::string &coreName, int index, int maxIndex)
    {
        std::string name = "ringlink_" + std::to_string(index);
        index_ = index;
        maxIndex_ = maxIndex;
        helics::FederateInfo fi(name);
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate>(fi);
        pub = vFed->registerPublicationIndexed<std::string>("pub", index_);
        if (index_ == 0)
        {
            sub = vFed->registerSubscriptionIndexed<std::string>("pub", maxIndex_-1);
        }
        else
        {
            sub = vFed->registerSubscriptionIndexed<std::string>("pub", index_-1);
        }
        initialized = true;
    }

    void mainLoop()
    {
        if (index_ == 0)
        {
            std::string txstring(100, '1');
            vFed->publish(pub, txstring);
        }
        auto nextTime = deltaTime;
        
        while (nextTime<finalTime)
        {

            nextTime = vFed->requestTime(finalTime);
            if (vFed->isUpdated(sub))
            {
                auto nstring = vFed->getValue<std::string>(sub);
                vFed->publish(pub,nstring);
            }

        }
        vFed->finalize();
    }
};

BOOST_AUTO_TEST_SUITE(ring_tests)

const int fedCount[] = {3,4};
//const int fedCount[] = { 180 };
#define CORE_TYPE_TO_TEST helics::core_type::TEST
BOOST_DATA_TEST_CASE(ring_test_single_core, bdata::make(fedCount), feds)
{
    auto wcore = helics::CoreFactory::FindOrCreate(CORE_TYPE_TO_TEST, "mcore", std::to_string(feds));
    //this is to delay until the threads are ready
    wcore->setFlagOption(helics::local_core_id, DELAY_INIT_ENTRY,true);
    std::vector<RingTransmit> links(feds);
    for (int ii = 0; ii < feds; ++ii)
    {
        links[ii].initialize("mcore", ii,feds);
    }

    std::vector<std::thread> threads(feds);
    for (int ii = 0; ii < feds; ++ii)
    {
        threads[ii] = std::thread([](RingTransmit &link) { link.run(); }, std::ref(links[ii]));
    }
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto startTime = std::chrono::high_resolution_clock::now();
    wcore->setFlagOption(helics::local_core_id, ENABLE_INIT_ENTRY,true);
    for (auto &thrd : threads)
    {
        thrd.join();
    }
    auto stopTime = std::chrono::high_resolution_clock::now();
    auto diff = stopTime - startTime;
    std::cout << feds << " feds total time=" << diff.count() / 1000000 << "ms \n";
}


const int fedCountB[] = { 5,5,5,5 };

BOOST_DATA_TEST_CASE(ring_test_multicores, bdata::make(fedCountB), feds)
{
    auto cType = helics::core_type::UDP;
    auto broker = helics::BrokerFactory::create(cType, "brokerb", std::to_string(feds + 1));
    auto wcore = helics::CoreFactory::FindOrCreate(cType, "mcore", "1");
    //this is to delay until the threads are ready
    wcore->setFlagOption(helics::local_core_id, DELAY_INIT_ENTRY,true);
    
    std::vector<RingTransmit> links(feds);
   links[0].initialize("mcore", 0, feds);
    std::vector<std::shared_ptr<helics::Core>> cores(feds);
    for (int ii = 1; ii < feds; ++ii)
    {
        cores[ii] = helics::CoreFactory::create(cType, "1");
        cores[ii]->connect();
        links[ii].initialize(cores[ii]->getIdentifier(), ii,feds);
    }

    std::vector<std::thread> threads(feds + 1);
    for (int ii = 0; ii < feds; ++ii)
    {
        threads[ii] = std::thread([](RingTransmit &link) { link.run(); }, std::ref(links[ii]));
    }
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::this_thread::yield();
    auto startTime = std::chrono::high_resolution_clock::now();
    wcore->setFlagOption(helics::local_core_id, ENABLE_INIT_ENTRY,true);
    for (auto &thrd : threads)
    {
        thrd.join();
    }
    auto stopTime = std::chrono::high_resolution_clock::now();
    auto diff = stopTime - startTime;
    std::cout << feds << " feds total time=" << diff.count() / 1000000 << "ms \n";

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    broker->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

BOOST_AUTO_TEST_SUITE_END()