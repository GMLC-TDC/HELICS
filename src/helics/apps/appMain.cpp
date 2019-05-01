/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../common/logger.h"
#include "../core/BrokerFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include "BrokerApp.hpp"
#include "Echo.hpp"
#include "Player.hpp"
#include "Recorder.hpp"
#include "Source.hpp"
#include "Tracer.hpp"
#include <iostream>

int main (int argc, char *argv[])
{
    helics::helicsCLI11App app ("helics_app", "");
    app.ignore_case ()->prefix_command ();
    app.add_subcommand ("player", "Helics Player App")->callback ([&app]() {
        helics::apps::Player player (app.remaining_for_passthrough (true));
        if (player.isActive ())
        {
            player.run ();
        }
    });

    app.add_subcommand ("recorder", "Helics Recorder App")->callback ([&app]() {
        helics::apps::Recorder recorder (app.remaining_for_passthrough (true));
        if (recorder.isActive ())
        {
            recorder.run ();
        }
    });
    app.add_subcommand ("echo", "Helics Echo App")->callback ([&app]() {
        helics::apps::Echo echo (app.remaining_for_passthrough (true));
        if (echo.isActive ())
        {
            echo.run ();
        }
    });

    app.add_subcommand ("source", "Helics Source App")->callback ([&app]() {
        helics::apps::Source source (app.remaining_for_passthrough (true));
        if (source.isActive ())
        {
            source.run ();
        }
    });

    app.add_subcommand ("tracer", "Helics Tracer App")->callback ([&app]() {
        helics::apps::Tracer tracer (app.remaining_for_passthrough (true));
        if (tracer.isActive ())
        {
            tracer.run ();
        }
    });

    app.add_subcommand ("broker", "Helics Broker App")->callback ([&app]() {
        helics::apps::BrokerApp broker (app.remaining_for_passthrough (true));
    });

    auto ret = app.helics_parse (argc, argv);
    helics::LoggerManager::getLoggerCore ()->addMessage ("!!>flush");

    helics::cleanupHelicsLibrary ();
    return (static_cast<int> (ret));
}
