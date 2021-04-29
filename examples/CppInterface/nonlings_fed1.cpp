/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <cmath>
#include <cpp98/Broker.hpp>
#include <cpp98/ValueFederate.hpp>
#include <cpp98/helics.hpp>  // helicsVersionString
#include <iostream>
#ifdef _MSC_VER
#    include <windows.h>
#else
#    include <unistd.h>
#endif

int main(int /*argc*/, char** /*argv*/)
{
    std::string initstring = "-f 2 --name=mainbroker";
    std::string fedinitstring = "--broker=mainbroker --federates=1";
    double deltat = 0.01;

    std::string helicsversion = helicscpp::getHelicsVersionString();
    if (helicsversion.find("error") == std::string::npos) {
        // this has to do with tests passing on CI builds
        std::cout << " Helics version = " << helicsversion << '\n';
    }

    /* Create broker */
    helicscpp::Broker broker("zmq", "", initstring);

    if (broker.isConnected()) {
        std::cout << " Broker created and connected\n";
    }

    /* Create Federate Info object that describes the federate properties
     * Set federate name and core type from string
     */
    helicscpp::FederateInfo fi("zmq");

    /* Federate init string */
    fi.setCoreInit(fedinitstring);
    fi.setProperty(helics_property_time_delta, deltat);
    fi.setProperty(helics_property_int_max_iterations, 100);

    // fi.setLoggingLevel(5);

    /* Create value federate */
    helicscpp::ValueFederate* vfed = new helicscpp::ValueFederate("TestA Federate", fi);
    std::cout << " Value federate created\n";

    /* Register the publication */
    helicscpp::Publication pub = vfed->registerGlobalPublication("testA", "double");
    std::cout << " Publication registered\n";
    /* Register the subscription */
    helicscpp::Input sub = vfed->registerSubscription("testB");
    std::cout << " Subscription registered\n";

    /* Enter initialization state */
    vfed->enterInitializingMode();  // can throw helicscpp::InvalidStateTransition exception
    std::cout << " Entered initialization state\n";

    double x = 0.0;
    double xprv = 100.0; /*yprv = 100,*/
    helics_time currenttime = 0.0;
    helicscpp::helics_iteration_time currenttimeiter;
    currenttimeiter.status = helics_iteration_result_iterating;
    // int isUpdated;
    double tol = 1E-8;

    pub.publish(x);
    /* Enter execution state */
    vfed->enterExecutingMode();  // can throw helicscpp::InvalidStateTransition exception
    std::cout << " Entered execution state\n";

    int helics_iter = 0;
    while (currenttimeiter.status == helics_iteration_result_iterating) {
        //    yprv = y;
        double y = sub.getDouble();
        bool newt_conv = false;
        int max_iter = 10;
        int iter = 0;
        /* Solve the equation using Newton */
        while (!newt_conv && iter < max_iter) {
            /* Function value */
            double f1 = x * x - 2 * x - y + 0.5;

            if (fabs(f1) < tol) {
                newt_conv = true;
            }
            iter++;

            /* Jacobian */
            double J1 = 2 * x - 2;

            x = x - f1 / J1;
        }
        ++helics_iter;
        std::cout << "Fed1: iteration " << helics_iter << " x=" << x << ", y=" << y << '\n';

        if ((fabs(x - xprv) > tol) || (helics_iter < 5)) {
            pub.publish(x);
            std::cout << "Fed1: publishing new x\n";
        }
        currenttimeiter =
            vfed->requestTimeIterative(currenttime, helics_iteration_request_iterate_if_needed);
        xprv = x;
    }

    std::cout << "NLIN1: Federate finalized\n";
    // Destructor for ValueFederate must be called before close library
    delete vfed;
    while (broker.isConnected()) {
#ifdef _MSC_VER
        Sleep(50);
#else
        usleep(50000); /* Sleep for 50 millisecond */
#endif
    }
    std::cout << "NLIN1: Broker disconnected\n";
    helicsCloseLibrary();
    std::cout << "NLIN1: Library closed" << std::endl;
    return (0);
}
