/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/ActionMessage.hpp"

using helics::operator"" _t;
// static constexpr helics::Time tend = 3600.0_t;  // simulation end time
/** class implementing the hub for an echo test*/
class EchoMessageHub {
  public:
    helics::Time finalTime = helics::Time(100, time_units::ms); // final time
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Endpoint ept;
    bool initialized = false;
    bool readyToRun = false;

  public:
    EchoMessageHub() = default;

    void run(std::function<void()> callOnReady = {})
    {
        if (!readyToRun) {
            makeReady();
        }
        if (callOnReady) {
            callOnReady();
        }
        mainLoop();
    };

    void initialize(const std::string& coreName)
    {
        std::string name = "echohub";
        helics::FederateInfo fi;
        fi.coreName = coreName;
        mFed = std::make_unique<helics::MessageFederate>(name, fi);
        ept = mFed->registerGlobalEndpoint("echo");
        initialized = true;
    }

    void makeReady()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        mFed->enterExecutingMode();
        readyToRun = true;
    }

    void mainLoop()
    {
        auto cTime = 0.0_t;
        while (cTime <= finalTime) {
            while (ept.hasMessage()) {
                auto m = ept.getMessage();
                std::swap(m->source, m->dest);
                std::swap(m->original_source, m->original_dest);
                ept.send(std::move(m));
            }

            cTime = mFed->requestTime(finalTime + 0.05);
        }
        mFed->finalize();
    }
};

class EchoMessageLeaf {
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Endpoint ept;

    int index_ = 0;
    bool initialized = false;
    bool readyToRun = false;

  public:
    EchoMessageLeaf() = default;

    void run(std::function<void()> callOnReady = {})
    {
        if (!readyToRun) {
            makeReady();
        }
        if (callOnReady) {
            callOnReady();
        }
        mainLoop();
    };
    void initialize(const std::string& coreName, int index)
    {
        std::string name = "echoleaf_" + std::to_string(index);
        index_ = index;
        helics::FederateInfo fi;
        fi.coreName = coreName;
        mFed = std::make_unique<helics::MessageFederate>(name, fi);
        // this is a local endpoint
        ept = mFed->registerEndpoint("leaf");
        initialized = true;
    }

    void makeReady()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        mFed->enterExecutingMode();
        readyToRun = true;
    }

    void mainLoop()
    {
        int cnt = 0;
        // this is  to make a fixed size string that is different for each federate but has sufficient length to
        // get beyond SSO
        const std::string txstring = std::to_string(100000 + index_) + std::string(100, '1');
        const int iter = 5000;
        while (cnt <= iter + 1) {
            mFed->requestNextStep();
            ++cnt;
            if (cnt <= iter) {
                ept.send("echo", txstring);
            }
            while (ept.hasMessage()) {
                auto m = ept.getMessage();
                auto& nstring = m->data.to_string();
                if (nstring != txstring) {
                    throw("incorrect string");
                }
            }
        }
        mFed->finalize();
    }
};