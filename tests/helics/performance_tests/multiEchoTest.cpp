/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/CoreFactory.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

using helics::operator"" _t;
helics::Time tend = 3600.0_t;  // simulation end time



/** class implementing a single heat transfer block*/
class EchoHub
{
public:
    helics::Time finalTime= helics::Time(1000, timeUnits::ns);  // final time
private:
    std::unique_ptr<helics::ValueFederate> vFed;
    std::vector<helics::publication_id_t> pubs;
    std::vector<helics::subscription_id_t> subs;
    int cnt_ = 10;
    bool initialized = false;

public:
    EchoHub() = default;

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

    void initialize(const std::string &coreName, int cnt)
    {
        cnt_ = cnt;
        std::string name = "echohub";
        helics::FederateInfo fi(name);
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate>(fi);
        pubs.reserve(cnt_);
        subs.reserve(cnt_);
        for (int ii = 0; ii < cnt_; ++ii)
        {
            pubs.push_back(vFed->registerPublicationIndexed < std::string > ("leafrx", ii));
            subs.push_back(vFed->registerRequiredSubscriptionIndexed<std::string>("leafsend", ii));
        }
        initialized = true;
    }

    void mainLoop()
    {
        auto cTime = 0.0_t;
        while (cTime < finalTime)
        {
            for (int ii = 0; ii < cnt_; ++ii)
            {
                if (vFed->isUpdated(subs[ii]))
                {
                    auto val = vFed->getValue<std::string>(subs[ii]);
                    vFed->publish(pubs[ii], val);
                }
            }
            cTime = vFed->requestTime(finalTime);
        }
        vFed->finalize();
    }
};

class EchoLeaf
{
public:
    helics::Time deltaTime = helics::Time(10, timeUnits::ns);  // sampling rate
    helics::Time finalTime = helics::Time(1000, timeUnits::ns);  // final time
private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::publication_id_t pub;
    helics::subscription_id_t sub;
    
    int index_ = 0;
    bool initialized = false;

public:
    EchoLeaf() = default;

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
    void initialize(const std::string &coreName, int index)
    {
        std::string name = "echoleaf_"+std::to_string(index);
        index_ = index;
        helics::FederateInfo fi(name);
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate>(fi);
        pub = vFed->registerPublicationIndexed<std::string>("leafsend",index_);
        sub = vFed->registerRequiredSubscriptionIndexed<std::string>("leafrx", index_);
        initialized = true;
    }

    void mainLoop()
    {
        auto nextTime = deltaTime;
        std::string txstring( 100,'1');
        while (nextTime<finalTime)
        {

            nextTime = vFed->requestTime(nextTime+deltaTime);
            vFed->publish(pub, txstring);
            if (vFed->isUpdated(sub))
            {
                auto nstring = vFed->getValue<std::string>(sub);
                if (nstring != txstring)
                {
                    std::cout << "incorrect string\n";
                    break;
                }
            }

        }
        vFed->finalize();
    }
};

BOOST_AUTO_TEST_SUITE(echo_tests)

#define FED_COUNT 5000
#define CORE_TYPE_TO_TEST helics::core_type::TEST
BOOST_AUTO_TEST_CASE(echo_test)
{
    auto wcore = helics::CoreFactory::FindOrCreate(CORE_TYPE_TO_TEST, "mcore", std::to_string(FED_COUNT+1));
    //this is to delay until the threads are ready
    wcore->setFlag(helics::invalid_fed_id, DELAY_INIT_ENTRY);
    EchoHub hub;
    hub.initialize("mcore",FED_COUNT);
    std::vector<EchoLeaf> leafs(FED_COUNT);
    for (int ii = 0; ii < FED_COUNT; ++ii)
    {
        leafs[ii].initialize("mcore",ii);
    }

    std::vector<std::thread> threads(FED_COUNT + 1);
    for (int ii = 0; ii < FED_COUNT; ++ii)
    {
        threads[ii] = std::thread([](EchoLeaf &leaf) { leaf.run(); }, std::ref(leafs[ii]));
    }
    threads[FED_COUNT] = std::thread([&]() { hub.run(); });
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto startTime = std::chrono::high_resolution_clock::now();
    wcore->setFlag(helics::invalid_fed_id, ENABLE_INIT_ENTRY);
    for (auto &thrd : threads)
    {
        thrd.join();
    }
    auto stopTime = std::chrono::high_resolution_clock::now();
    auto diff = stopTime - startTime;
    std::cout << "total time=" << diff.count()/1000000 << "ms \n";
}

BOOST_AUTO_TEST_SUITE_END()