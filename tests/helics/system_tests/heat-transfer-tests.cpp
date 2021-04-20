/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC. See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/CoreFactory.hpp"

#include "gtest/gtest.h"
#include <fstream>
#include <thread>

using helics::operator"" _t;
helics::Time tend = 3600.0_t;  // simulation end time

/** class implementing a single heat transfer block*/
class HeatUnitBlock {
  public:
    double ThermalCap = 4.0;  // thermal capacity
    double tRate = 0.05;  // rate of energy transfer per degree
    int x = 0;  // x coordinate
    int y = 0;  // y coordinate

    double T = 25.0;  // temperature
    helics::Time deltaTime = 5.0;  // sampling rate
  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::Publication* pub;
    helics::Input* sub[4];
    bool initialized = false;

  public:
    HeatUnitBlock() = default;

    void run(const std::string& coreName = "")
    {
        if (!initialized) {
            initialize(coreName);
        }

        vFed->enterInitializingMode();
        vFed->publish(*pub, T);
        vFed->enterExecutingMode();
        mainLoop();
    };
    void initialize(const std::string& coreName)
    {
        std::string name = "heatUnit_(" + std::to_string(x) + "," + std::to_string(y) + ")";
        helics::FederateInfo fi;
        fi.coreName = coreName;
        fi.setProperty(helics_property_time_delta, deltaTime);
        fi.setFlagOption(helics_handle_option_connection_optional);
        vFed = std::make_unique<helics::ValueFederate>(name, fi);
        pub = &vFed->registerIndexedPublication<double>("temp", x, y);
        if (x - 1 < 0) {
            sub[0] = &vFed->registerSubscription("temp_wall");
        } else {
            sub[0] = &vFed->registerIndexedSubscription("temp", x - 1, y);
        }
        sub[0]->setDefault(T);
        sub[1] = &vFed->registerIndexedSubscription("temp", x + 1, y);
        sub[1]->setDefault(-512.0);

        sub[2] = &vFed->registerIndexedSubscription("temp", x, y - 1);
        sub[2]->setDefault(-512.0);
        sub[3] = &vFed->registerIndexedSubscription("temp", x, y + 1);
        sub[3]->setDefault(-512.0);
        initialized = true;
    }

    void mainLoop()
    {
        auto cTime = 0.0_t;
        while (cTime < tend) {
            auto T0 = sub[0]->getValue<double>();

            double Etransfer = (T0 - T) * tRate * deltaTime;

            for (int ii = 1; ii < 3; ++ii) {
                auto TT = sub[ii]->getValue<double>();
                if (TT > -500) {
                    Etransfer += (TT - T) * tRate * deltaTime;
                }
            }
            double deltaT = Etransfer / ThermalCap;
            T = T + deltaT;
            vFed->publish(*pub, T);
            cTime = vFed->requestTime(cTime + deltaTime);
        }
        vFed->finalize();
    }
};

class Wall {
    double Temp = 80;  // thermal capacity
    std::vector<std::pair<helics::Time, double>> schedTemp;

  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::Publication* pub = nullptr;
    int index = 0;
    bool initialized = false;

  public:
    Wall() = default;

    void run(const std::string& coreName = "")
    {
        if (!initialized) {
            initialize(coreName);
        }
        vFed->enterInitializingMode();
        vFed->publish(*pub, Temp);
        vFed->enterExecutingMode();
        vFed->publish(*pub, Temp);
        mainLoop();
    };
    void initialize(const std::string& coreName)
    {
        std::string name = "Wall";
        helics::FederateInfo fi;
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate>(name, fi);
        pub = &vFed->registerGlobalPublication<double>("temp_wall");
        initialized = true;
    }

    void addTemp(helics::Time timechange, double newTemp)
    {
        schedTemp.emplace_back(timechange, newTemp);
    }
    void mainLoop()
    {
        auto nextTime = tend;
        helics::Time retTime = 0;
        if (!schedTemp.empty()) {
            nextTime = schedTemp[0].first;
        }
        do {
            retTime = vFed->requestTime(nextTime);
            if (index < static_cast<int>(schedTemp.size())) {
                vFed->publish(*pub, schedTemp[index].second);
                nextTime = schedTemp[index].first;
                ++index;
            } else {
                nextTime = tend;
            }

        } while (retTime < tend);
        vFed->finalize();
    }
};

class observer {
  public:
    std::vector<std::vector<double>> vals;
    std::vector<helics::Time> times;

  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::VectorSubscription2d<double> vSub;
    std::string subName;
    int index = 0;
    int m_count = 20;
    bool initialized = false;

  public:
    observer(std::string name, int count): subName(std::move(name)), m_count(count) {}
    void run(const std::string& coreName = "")
    {
        if (!initialized) {
            initialize(coreName);
        }
        vFed->enterExecutingMode();
        mainLoop();
    };
    void initialize(const std::string& coreName)
    {
        std::string name = "observer";
        helics::FederateInfo fi;
        fi.coreName = coreName;
        fi.setFlagOption(helics_flag_observer);
        fi.setProperty(helics_property_time_delta, 10.0);
        vFed = std::make_unique<helics::ValueFederate>(name, fi);
        vSub = helics::VectorSubscription2d<double>(vFed.get(), subName, 0, m_count, 0, 1, 0.0);
        initialized = true;
    }

    void mainLoop()
    {
        helics::Time nextTime = 0.0;
        do {
            vals.push_back(vSub.getVals());
            times.push_back(nextTime);
            ++index;
            nextTime = vFed->requestTime(nextTime + 10.0);

        } while (nextTime < tend);
        vFed->finalize();
    }
    void saveFile(const std::string& fileName) const
    {
        std::ofstream out(fileName);
        for (int ii = 0; ii < static_cast<int>(times.size()); ++ii) {
            out << static_cast<double>(times[ii]);
            for (auto& val : vals[ii]) {
                out << ", " << val;
            }
            out << '\n';
        }
    }
};

TEST(heat_transfer_tests, linear_tests)
{
    auto wcore = helics::CoreFactory::FindOrCreate(helics::core_type::TEST,
                                                   "wallcore",
                                                   "-f 22 --autobroker");
    Wall w;
    w.initialize("wallcore");
    int blockCount = 20;
    std::vector<HeatUnitBlock> block(blockCount);
    for (int ii = 0; ii < blockCount; ++ii) {
        block[ii].x = ii;
        block[ii].initialize("wallcore");
    }
    observer obs("temp", blockCount);
    obs.initialize("wallcore");

    std::vector<std::thread> threads(static_cast<size_t>(blockCount) + 2);
    for (int ii = 0; ii < blockCount; ++ii) {
        threads[ii] = std::thread([](HeatUnitBlock& blk) { blk.run(); }, std::ref(block[ii]));
    }
    threads[blockCount] = std::thread([&]() { obs.run(); });
    threads[static_cast<size_t>(blockCount) + 1] = std::thread([&]() { w.run(); });
    for (auto& thrd : threads) {
        thrd.join();
    }
    obs.saveFile("tempData.csv");
}
