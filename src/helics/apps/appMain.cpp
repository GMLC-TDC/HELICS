/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

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
    helics::helicsCLI11App app ("", "helics_app");
    app.ignore_case ();
    app.prefix_command ();
    app.add_subcommand ("Helics Player App", "player")->prefix_command ()->callback ([&app]() {
        helics::apps::Player player (app.remaining_args ());
        if (player.isActive ())
        {
            player.run ();
        }
    });

    app.add_subcommand ("Helics Recorder App", "recorder")->prefix_command ()->callback ([&app]() {
        helics::apps::Recorder recorder (app.remaining_args ());
        if (recorder.isActive ())
        {
            recorder.run ();
        }
    });
    app.add_subcommand ("Helics Echo App", "echo")->prefix_command ()->callback ([&app]() {
        helics::apps::Echo echo (app.remaining_args ());
        if (echo.isActive ())
        {
            echo.run ();
        }
    });

    app.add_subcommand ("Helics Source App", "source")->prefix_command ()->callback ([&app]() {
        helics::apps::Source source (app.remaining_args ());
        if (source.isActive ())
        {
            source.run ();
        }
    });

    app.add_subcommand ("Helics Tracer App", "tracer")->prefix_command ()->callback ([&app]() {
        helics::apps::Tracer tracer (app.remaining_args ());
        if (tracer.isActive ())
        {
            tracer.run ();
        }
    });

    app.add_subcommand ("Helics Broker App", "broker")->prefix_command ()->callback ([&app]() {
        helics::apps::BrokerApp broker (app.remaining_args ());
    });

    auto ret = app.helics_parse (argc, argv);

    helics::cleanupHelicsLibrary ();
    return (static_cast<int> (ret));
}
