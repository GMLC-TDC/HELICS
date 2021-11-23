#pragma once
#include "BenchmarkFederate.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/queryFunctions.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/Core.hpp"

#include <algorithm>
#include <random>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <utility>
#include <vector>


/* class implementing nodes within a Barabasi-Albert network*/
class BarabasiAlbertFederate: public BenchmarkFederate {
 public:
    int initialMessageCount = 10;   //number of messages the federate should send when it starts 

  private:
    helics::Endpoint* ept = nullptr;
    std::string targets;
    std::string sb;
    std::vector<std::string> targets_vector;

  public:
    BarabasiAlbertFederate(): BenchmarkFederate("BarabasiAlbertFederate") {}
    std::string getName() override { return getNameForIndex(index); }
    static std::string getNameForIndex(int index) { return "BA_" + std::to_string(index); }


    void setupArgumentParsing() override
    {
        deltaTime = helics::Time(10, time_units::ns);
        finalTime = helics::Time(5000, time_units::ns);
        opt_index->required();
        opt_max_index->required();
        app->add_option("--targets", targets, "targets this node federate will form links with");
    }

    void doFedInit() override {
        ept = &fed->registerTargetedEndpoint("ept");

        if (targets.compare("None") == 0) {
            return;
        }

        auto cp = fed->getCorePointer();

        std::stringstream ss(targets);

        while (std::getline(ss, sb, ',')) {
            targets_vector.push_back(sb);
        }
        for (size_t i = 0; i < targets_vector.size(); i++) {
            cp->linkEndpoints(ept->getName(), targets_vector[i]);  
        }
    }
    void doMakeReady() override
    {
        // send initial messages
        std::string txstring(100, '1');
        for (int i = 0; i < initialMessageCount; i++) {
            ept->send(txstring);
        }
    }
    void doMainLoop() override
    {
       auto nextTime = helics::timeZero;

       while (nextTime < finalTime) {
            nextTime = fed->requestTime(finalTime);     
            while (ept->hasMessage()) {
                auto m = fed->getMessage(*ept);
                m->source = ept->getName();
                m->time = fed->getCurrentTime() + deltaTime;
                ept->send(std::move(m));
            }
        }
    }
};
