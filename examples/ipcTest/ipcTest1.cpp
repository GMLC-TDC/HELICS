/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

#include <helics/application_api/ValueFederate.hpp>
#include <helics/core/BrokerFactory.hpp>
#include <helics/core/CoreBroker.hpp>

#include "common.hpp"

// TestA will send doubles
using ValueSetter = ValuePacket<double>;

// TestB will send ints
using ValueRecver = ValuePacket<int>;

void sendPublication (helics::ValueFederate &vFed, ValueSetter const &vs);

int main (int, char **)
{
    constexpr unsigned num_tsteps = 10;
    const double base_time = 0.0;
    const double delta_t = 0.1;

    std::cout << "trying to create broker..." << std::endl;

    auto init_string = std::string ("2 --name=stevebroker");
    auto broker = helics::BrokerFactory::create (helics::core_type::INTERPROCESS, init_string);

    std::cout << "created broker \"" << broker->getIdentifier () << "\"\n"
              << "broker is connected: " << std::boolalpha << broker->isConnected () << std::endl;

    std::mt19937_64 gen (std::random_device{}());
    std::uniform_real_distribution<double> dist (0.0, std::nextafter (10.0, std::numeric_limits<double>::max ()));

    std::ofstream ofs ("TestA.log");

    helics::FederateInfo fed_info ("TestA Federate");
    fed_info.coreType = helics::core_type::IPC;
    fed_info.coreInitString = "--broker=stevebroker --federates 1";
    fed_info.timeDelta = delta_t;
    fed_info.logLevel = 5;
    helics::ValueFederate fed (fed_info);

    auto id = fed.registerGlobalPublication ("testA", "double");

    fed.enterExecutionState ();

    for (unsigned tstep = 0; tstep < num_tsteps; ++tstep)
    {
        const double this_time = base_time + tstep * delta_t;
        const double this_value = dist (gen);

        auto thisTime = fed.requestTime (helics::Time (this_time));

        // Output to stdout
        std::cout << "welcome to timestep " << tstep << '\n'
                  << "   x(" << this_time << ") = " << this_value << '\n'
                  << "   sending...";

        // Output to log file
        ofs << std::setw (10) << std::right << this_time << std::setw (10) << std::right << this_value
            << std::endl;

        sendPublication (fed, ValueSetter (thisTime, id, this_value));

        std::cout << "done." << std::endl;
    }

    fed.finalize ();
    do  // sleep until the broker finishes
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (500));

    } while (broker->isConnected ());

    return 0;
}

void sendPublication (helics::ValueFederate &vFed, ValueSetter const &vs) { vFed.publish (vs.id_, vs.value_); }
