/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BenchmarkFederate.hpp"
#include "helics/application_api/Inputs.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/core/ActionMessage.hpp"

using helics::operator"" _t;
using namespace helics;
/** class implementing the hub for a timing test*/
class TimingHub : public BenchmarkFederate {
  private:
    std::vector<helics::Publication> pubs;
    std::vector<helics::Input> subs;
    int cnt = 10;

  public:
    TimingHub() : BenchmarkFederate("timing hub benchmark federate") {}

    std::string getName() override
    {
        return "echohub";
    }

    void setupArgumentParsing() override
    {
        finalTime = helics::Time(100, time_units::ms);
    
        app->add_option("--num_leafs", cnt, "the number of echoleaf federates to expect")->required();
    }

    void doFedInit() override
    {
        pubs.reserve(cnt);
        subs.reserve(cnt);
        for (int ii = 0; ii < cnt; ++ii) {
            pubs.push_back(fed->registerPublicationIndexed<std::string>("leafrx", ii));
            subs.push_back(fed->registerSubscriptionIndexed("leafsend", ii));
        }
    }

    void doMainLoop() override
    {
        auto cTime = 0.0_t;
        while (cTime <= finalTime) {
            cTime = fed->requestTime(finalTime + 0.05);
        }
    }
};
