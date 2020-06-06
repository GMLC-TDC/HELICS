/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC. See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "MpiCore.h"

#include "../../core/helicsCLI11.hpp"
#include "MpiComms.h"

#include <memory>
#include <mpi.h>
#include <string>

namespace helics {
namespace mpi {
    MpiCore::MpiCore() noexcept {}

    // MpiCore::~MpiCore () = default;
    MpiCore::~MpiCore()
    { /*std::cout << "MpiCore destructor for " << MpiCore::getAddress () << std::endl;*/
    }
    MpiCore::MpiCore(const std::string& core_name): CommsBroker(core_name) {}

    std::shared_ptr<helicsCLI11App> MpiCore::generateCLI()
    {
        auto hApp = CommonCore::generateCLI();
        hApp->description("Message Passing Interface Core operation command line arguments");
        hApp->add_option_function<std::string>(
                "--broker_address,--broker",
                [this](const std::string& addr) {
                    auto delim_pos = addr.find_first_of(':', 1);
                    try {
                        brokerRank = std::stoi(addr.substr(0, delim_pos));
                        brokerTag = std::stoi(addr.substr(delim_pos + 1, addr.length()));
                    }
                    catch (const std::invalid_argument&) {
                        throw(CLI::ValidationError("address does not evaluate to integers"));
                    }
                },
                "location of a broker using mpi (rank:tag)")
            ->ignore_underscore();
        hApp->add_option("--broker_rank,--rank", brokerRank, "mpi rank of a broker using mpi")
            ->ignore_underscore();
        hApp->add_option("--broker_tag,--tag", brokerTag, "mpi tag of a broker using mpi")
            ->ignore_underscore();
        hApp->add_callback([this]() {
            brokerAddress = std::to_string(brokerRank) + ":" + std::to_string(brokerTag);
        });
        return hApp;
    }

    bool MpiCore::brokerConnect()
    {
        std::lock_guard<std::mutex> lock(
            dataMutex);  // mutex protecting the other information in the ipcBroker

        if (brokerAddress.empty()) {
            brokerAddress = "0:0";
        }
        comms->setBrokerAddress(brokerAddress);

        comms->setName(getIdentifier());

        return comms->connect();
    }

    std::string MpiCore::generateLocalAddressString() const { return comms->getAddress(); }

}  // namespace mpi
}  // namespace helics
