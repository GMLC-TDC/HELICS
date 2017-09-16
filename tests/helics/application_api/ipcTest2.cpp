#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

#include <helics/core/BrokerFactory.h>
#include <helics/application_api/ValueFederate.h>

#include "common.hpp"

// TestA will send doubles
using ValueSetter = ValuePacket<double>;

// TestB will send ints
using ValueRecver = ValuePacket<int>;

void sendPublication (helics::ValueFederate& vFed, ValueSetter const& vs);

void break_on_me(void) {}

int main(int, char**)
{
    std::ofstream ofs("TestB.log");
    helics::Time stopTime = helics::Time(0.9);

    helics::FederateInfo fed_info("TestB Federate");
    fed_info.coreType = "ipc";
    fed_info.coreInitString = "--broker=stevebroker --federates 1";
    fed_info.timeDelta = 0.1;

    fed_info.observer=false;

    std::cout << "Creating federate." << std::endl;
    helics::ValueFederate fed(fed_info);
    std::cout << "Done creating federate." << std::endl;

    // Subscribe to testA's publications
    auto id = fed.registerRequiredSubscription("testA","double");

    fed.enterExecutionState();

    break_on_me();

    std::cout << "Updated? " << std::boolalpha << fed.isUpdated(id)
              << std::endl;

    unsigned tstep = 0;
    for (;;)
    {
        auto time = fed.requestTime(stopTime);
        std::cout << "at time " << time << std::endl;
        if (time <= stopTime)
        {
            if (fed.isUpdated(id))
            {
                auto this_value = fed.getValue<double>(id);
                std::cout << "welcome to timestep " << ++tstep << '\n'
                          << "   x(" << time << ") = " << this_value
                          << std::endl;

                ofs << std::setw(10) << std::right << time
                    << std::setw(10) << std::right << this_value << std::endl;
            }
        }
        else
            break;
    }

    fed.finalize();
    return 0;
}

void sendPublication (helics::ValueFederate& vFed, ValueSetter const& vs)
{
    vFed.publish(vs.id_,vs.value_);
}
