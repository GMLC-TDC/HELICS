/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MpiBroker.h"
#include "MpiComms.h"

#include "../../common/argParser.h"

#include <mpi.h>

namespace helics
{
    MpiBroker::MpiBroker(bool rootBroker) noexcept : CommsBroker(rootBroker) {}

    MpiBroker::MpiBroker(const std::string &broker_name) : CommsBroker(broker_name) {}

    MpiBroker::~MpiBroker() = default;

    using namespace std::string_literals;
    static const ArgDescriptors extraArgs{ { "broker_rank", "int"s, "mpi rank of the broker" } };

    void MpiBroker::displayHelp(bool local_only)
    {
        std::cout << " Help for MPI Broker: \n";
        namespace po = boost::program_options;
        po::variables_map vm;
        const char *const argV[] = { "", "--help" };
        argumentParser(2, argV, vm, extraArgs);
        if (!local_only)
        {
            CoreBroker::displayHelp();
        }
    }

    void MpiBroker::initializeFromArgs(int argc, const char *const *argv)
    {
        if (brokerState == broker_state_t::created)
        {
            namespace po = boost::program_options;
            po::variables_map vm;
            argumentParser(argc, argv, vm, extraArgs);

            if (vm.count("broker_rank") > 0)
            {
                brokerRank = vm["broker_rank"].as<int>();
            }

            CoreBroker::initializeFromArgs(argc, argv);
        }
    }

    bool MpiBroker::brokerConnect()
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        if (brokerRank == -1)
        {
            setAsRoot();
        }
        comms = std::make_unique<MpiComms>(brokerRank);
        comms->setCallback([this](ActionMessage M) { addActionMessage(std::move(M)); });
        comms->setName(getIdentifier());

        if (getAddress().compare(std::to_string(brokerRank)) == 0)
        {
            setAsRoot();
        }

    auto res = comms->connect ();
    return res;
}

std::string MpiBroker::getAddress () const
{
    return MpiComms::getAddress();
}
}  // namespace helics
