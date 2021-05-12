/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/BrokerApp.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include "Clone.hpp"
#include "Echo.hpp"
#include "Player.hpp"
#include "Recorder.hpp"
#include "Source.hpp"
#include "Tracer.hpp"

#include <iostream>
#include <spdlog/logger.h>

static const std::vector<std::string> helpArgs{"-?"};
int main(int argc, char* argv[])
{
    helics::helicsCLI11App app("simple execution for all the different HELICS apps", "helics_app");
    app.ignore_case()->prefix_command();
    app.add_subcommand("player", "Helics Player App")
        ->callback([&app]() {
            helics::apps::Player player(app.remaining_for_passthrough(true));
            std::cout << "player subcommand\n";
            if (player.isActive()) {
                player.run();
            }
        })
        ->footer([] {
            helics::apps::Player player({"-?"});
            return std::string{};
        });

    app.add_subcommand("recorder", "Helics Recorder App")
        ->callback([&app]() {
            helics::apps::Recorder recorder(app.remaining_for_passthrough(true));
            std::cout << "recorder subcommand\n";
            if (recorder.isActive()) {
                recorder.run();
            }
        })
        ->footer([] {
            helics::apps::Recorder rec({"-?"});
            return std::string{};
        });
    app.add_subcommand("clone", "Helics Clone App")
        ->callback([&app]() {
            helics::apps::Clone cloner(app.remaining_for_passthrough(true));
            std::cout << "clone subcommand\n";
            if (cloner.isActive()) {
                cloner.run();
            }
        })
        ->footer([] {
            helics::apps::Clone rec({"-?"});
            return std::string{};
        });
    app.add_subcommand("echo", "Helics Echo App")
        ->callback([&app]() {
            std::cout << "echo subcommand\n";
            helics::apps::Echo echo(app.remaining_for_passthrough(true));
            if (echo.isActive()) {
                echo.run();
            }
        })
        ->footer([] {
            helics::apps::Echo echo({"-?"});
            return std::string{};
        });

    app.add_subcommand("source", "Helics Source App")
        ->callback([&app]() {
            std::cout << "source subcommand\n";
            helics::apps::Source source(app.remaining_for_passthrough(true));
            if (source.isActive()) {
                source.run();
            }
        })
        ->footer([] {
            helics::apps::Source src({"-?"});
            return std::string{};
        });

    app.add_subcommand("tracer", "Helics Tracer App")
        ->callback([&app]() {
            std::cout << "tracer subcommand\n";
            helics::apps::Tracer tracer(app.remaining_for_passthrough(true));
            if (tracer.isActive()) {
                tracer.run();
            }
        })
        ->footer([] {
            helics::apps::Tracer trac({"-?"});
            return std::string{};
        });

    app.add_subcommand("broker", "Helics Broker App")
        ->callback([&app]() {
            std::cout << "broker subcommand\n";
            helics::BrokerKeeper broker(app.remaining_for_passthrough(true));
        })
        ->footer([=] {
            helics::BrokerApp broker(argc, argv);
            return std::string{};
        });
    app.footer(
        "helics_app [SUBCOMMAND] --help will display the options for a particular subcommand");
    auto ret = app.helics_parse(argc, argv);

    helics::cleanupHelicsLibrary();

    switch (ret) {
        case helics::helicsCLI11App::parse_output::help_call:
        case helics::helicsCLI11App::parse_output::help_all_call:
        case helics::helicsCLI11App::parse_output::version_call:
        case helics::helicsCLI11App::parse_output::ok:
            return 0;
        default:
            return static_cast<int>(ret);
    }
}
