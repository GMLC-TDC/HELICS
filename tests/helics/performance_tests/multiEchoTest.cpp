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

using helics::operator"" _t;
static constexpr helics::Time tend = 3600.0_t;  // simulation end time
namespace bdata = boost::unit_test::data;


/** class implementing the hub for an echo test*/
class EchoHub
{
public:
    helics::Time finalTime= helics::Time(1000, timeUnits::ns);  // final time
private:
    std::unique_ptr<helics::ValueFederate> vFed;
    std::vector<helics::publication_id_t> pubs;
    std::vector<helics::input_id_t> subs;
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
        vFed->enterInitializingMode();
        vFed->enterExecutingMode();
        mainLoop();
    };

    void initialize(const std::string &coreName, int cnt)
    {
        cnt_ = cnt;
        std::string name = "echohub";
        helics::FederateInfo fi;
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate>(name,fi);
        pubs.reserve(cnt_);
        subs.reserve(cnt_);
        for (int ii = 0; ii < cnt_; ++ii)
        {
            pubs.push_back(vFed->registerPublicationIndexed < std::string > ("leafrx", ii));
            subs.push_back(vFed->registerSubscriptionIndexed<std::string>("leafsend", ii));
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
    helics::input_id_t sub;
    
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
        vFed->enterInitializingMode();
        vFed->enterExecutingMode();
        mainLoop();
    };
    void initialize(const std::string &coreName, int index)
    {
        std::string name = "echoleaf_"+std::to_string(index);
        index_ = index;
        helics::FederateInfo fi;
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate>(name,fi);
        pub = vFed->registerPublicationIndexed<std::string>("leafsend",index_);
        sub = vFed->registerSubscriptionIndexed<std::string>("leafrx", index_);
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

const int fedCount[] = { 1,5,10,20,40,80,160,320,500,1000,2000 };
//const int fedCount[] = {180};
#define CORE_TYPE_TO_TEST helics::core_type::TEST
BOOST_DATA_TEST_CASE(echo_test_single_core,bdata::make(fedCount),feds)
{
    auto wcore = helics::CoreFactory::FindOrCreate(CORE_TYPE_TO_TEST, "mcore", std::to_string(feds+1));
    //this is to delay until the threads are ready
    wcore->setFlagOption(helics::local_core_id, DELAY_INIT_ENTRY,true);
    EchoHub hub;
    hub.initialize("mcore",feds);
    std::vector<EchoLeaf> leafs(feds);
    for (int ii = 0; ii < feds; ++ii)
    {
        leafs[ii].initialize("mcore",ii);
    }

    std::vector<std::thread> threads(feds + 1);
    for (int ii = 0; ii < feds; ++ii)
    {
        threads[ii] = std::thread([](EchoLeaf &leaf) { leaf.run(); }, std::ref(leafs[ii]));
    }
    threads[feds] = std::thread([&]() { hub.run(); });
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
    std::cout << feds <<" feds total time=" << diff.count()/1000000 << "ms \n";
}


const int fedCountB[] = {5,5,5,5};

BOOST_DATA_TEST_CASE(echo_test_multicores, bdata::make(fedCountB), feds)
{
    auto cType = helics::core_type::TEST;
    auto broker = helics::BrokerFactory::create(cType,"brokerb", std::to_string(feds+1));
    auto wcore = helics::CoreFactory::FindOrCreate(cType, "mcore", "1");
    //this is to delay until the threads are ready
    wcore->setFlagOption(helics::local_core_id, DELAY_INIT_ENTRY,true);
    EchoHub hub;
    hub.initialize("mcore", feds);
    std::vector<EchoLeaf> leafs(feds);
    std::vector<std::shared_ptr<helics::Core>> cores(feds);
    for (int ii = 0; ii < feds; ++ii)
    {
        cores[ii] = helics::CoreFactory::create(cType, "1");
        cores[ii]->connect();
        leafs[ii].initialize(cores[ii]->getIdentifier(), ii);
    }

    std::vector<std::thread> threads(feds + 1);
    for (int ii = 0; ii < feds; ++ii)
    {
        threads[ii] = std::thread([](EchoLeaf &leaf) { leaf.run(); }, std::ref(leafs[ii]));
    }
    threads[feds] = std::thread([&]() { hub.run(); });
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
    std::cout << feds << " feds total time=" << diff.count() / 1000000 << "ms "<<std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    broker->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

BOOST_AUTO_TEST_SUITE_END()