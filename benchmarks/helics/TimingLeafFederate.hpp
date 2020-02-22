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
/** class implementing the leaf for a timing test*/
class TimingLeaf: public BenchmarkFederate {
  private:
    helics::Publication pub;
    helics::Input sub;

  public:
    TimingLeaf(): BenchmarkFederate("timing leaf benchmark federate") {}

    std::string getName() override { return "echoleaf_" + std::to_string(index); }

    void setupArgumentParsing() override { opt_index->required(); }

    void doFedInit() override
    {
        pub = fed->registerPublicationIndexed<std::string>("leafsend", index);
        sub = fed->registerSubscriptionIndexed("leafrx", index);
    }

    void doMainLoop() override
    {
        int cnt = 0;
        // this is  to make a fixed size string that is different for each federate but has sufficient length to
        // get beyond SSO
        const int iter = 5000;
        while (cnt <= iter + 1) {
            fed->requestNextStep();
            ++cnt;
        }
    }
};
