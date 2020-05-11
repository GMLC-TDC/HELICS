/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC. See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "MpiBroker.h"

#include "../../core/helicsCLI11.hpp"
#include "MpiComms.h"

#include <memory>
#include <mpi.h>
#include <string>

namespace helics {
namespace mpi {
    MpiBroker::MpiBroker(bool rootBroker) noexcept: CommsBroker(rootBroker) {}

    MpiBroker::MpiBroker(const std::string& broker_name): CommsBroker(broker_name) {}

    // MpiBroker::~MpiBroker() = default;
    MpiBroker::~MpiBroker()
    {
        std::cout << "MpiBroker destructor for " << getAddress() << std::endl;
    }

    std::shared_ptr<helicsCLI11App> MpiBroker::generateCLI()
    {
        auto hApp = CoreBroker::generateCLI();
        hApp->description("Message Passing Interface Broker command line arguments");
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

    bool MpiBroker::brokerConnect()
    {
        std::lock_guard<std::mutex> lock(
            dataMutex);  // mutex protecting the other information in the ipcBroker

        if (brokerAddress.empty()) {
            setAsRoot();
        } else {
            comms->setBrokerAddress(brokerAddress);
        }

        comms->setName(getIdentifier());

        return comms->connect();
    }

    std::string MpiBroker::generateLocalAddressString() const { return comms->getAddress(); }
}  // namespace mpi
}  // namespace helics
