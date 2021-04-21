/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BenchmarkFederate.hpp"
#include "EchoHubFederate.hpp"
#include "EchoLeafFederate.hpp"
#include "EchoMessageHubFederate.hpp"
#include "EchoMessageLeafFederate.hpp"
#include "MessageExchangeFederate.hpp"
#include "PholdFederate.hpp"
#include "RingTransmitFederate.hpp"
#include "RingTransmitMessageFederate.hpp"
#include "TimingHubFederate.hpp"
#include "TimingLeafFederate.hpp"
#include "WattsStrogatzFederate.hpp"
#include "helics/core/helicsCLI11.hpp"
#include "helics_benchmark_util.h"

#include <chrono>
#include <iostream>

std::shared_ptr<BenchmarkFederate> fed;

template<class T>
void addBM(helics::helicsCLI11App& app, std::string name, std::string description)
{
    try {
        app.add_subcommand(std::move(name), std::move(description))
            ->callback([]() { fed = std::make_shared<T>(); })
            ->footer([] {
                T().initialize("", "--help");
                return std::string{};
            });
    }
    catch (...) {
        std::cerr << "Exception when adding CLI11 subcommand occurred\n";
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    try {
        helics::helicsCLI11App app(
            "HELICS benchmark federates for use in multinode benchmark setups",
            "helics_benchmarks");
        app.ignore_case()->prefix_command()->ignore_underscore();

        // add a flag for printing system info
        app.add_flag_callback(
            "--print_systeminfo",
            []() {
                printHELICSsystemInfo();
                exit(0);
            },
            "prints the HELICS system info and exits");

        addBM<EchoHub>(app, "echohub", "Echo Hub benchmark federate");
        addBM<EchoLeaf>(app, "echoleaf", "Echo Leaf benchmark federate");
        addBM<EchoMessageHub>(app, "echomessagehub", "Echo Message Hub benchmark federate");
        addBM<EchoMessageLeaf>(app, "echomessageleaf", "Echo Message Leaf benchmark federate");

        addBM<MessageExchangeFederate>(app,
                                       "messageexchange",
                                       "Message Exchange benchmark federate");

        addBM<PholdFederate>(app, "phold", "PHOLD benchmark federate");

        addBM<RingTransmit>(app, "ringtransmit", "Ring Transmit benchmark federate");
        addBM<RingTransmitMessage>(app,
                                   "ringtransmitmessage",
                                   "Ring Transmit Message benchmark federate");

        addBM<TimingHub>(app, "timinghub", "Timing Hub benchmark federate");
        addBM<TimingLeaf>(app, "timingleaf", "Timing Leaf benchmark federate");
        addBM<WattsStrogatzFederate>(app, "watts-strogatz", "Watts-Strogatz benchmark federate");

        auto ret = app.helics_parse(argc, argv);
        if (ret != helics::helicsCLI11App::parse_output::ok) {
            switch (ret) {
                case helics::helicsCLI11App::parse_output::help_call:
                case helics::helicsCLI11App::parse_output::help_all_call:
                case helics::helicsCLI11App::parse_output::version_call:
                    return 0;
                default:
                    return static_cast<int>(ret);
            }
        }

        helics::FederateInfo fi;
        int rc = fed->initialize(fi, app.remaining_for_passthrough(true));
        if (rc != 0) {
            exit(rc);
        }

        // setup benchmark timing
        std::chrono::time_point<std::chrono::steady_clock> start_time;
        std::chrono::time_point<std::chrono::steady_clock> end_time;
        fed->setBeforeFinalizeCallback(
            [&end_time]() { end_time = std::chrono::steady_clock::now(); });

        // run the benchmark
        try {
            fed->run([&start_time]() { start_time = std::chrono::steady_clock::now(); });
        }
        catch (...) {
            std::cerr << "Exception occurred while running the benchmark\n";
            exit(1);
        }

        // print duration
        auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
        fed->addResult<decltype(elapsed)>("ELAPSED TIME (ns)", "real_time", elapsed);
        fed->printResults();
    }
    catch (...) {
        std::cerr << "Uncaught exception occurred\n";
        exit(1);
    }
}
