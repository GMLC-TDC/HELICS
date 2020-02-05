/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <cpp98/Broker.hpp>
#include <cpp98/ValueFederate.hpp>
#include <cpp98/helics.hpp> // helicsVersionString
#include <math.h>
#include <stdio.h>
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
    helicscpp::Publication pub;
    helicscpp::Input sub;

    std::string helicsversion = helicscpp::getHelicsVersionString();

    printf(" Helics version = %s\n", helicsversion.c_str());

    /* Create broker */
    helicscpp::Broker broker("zmq", "", initstring);

    if (broker.isConnected()) {
        printf(" Broker created and connected\n");
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
    printf(" Value federate created\n");

    /* Register the publication */
    pub = vfed->registerGlobalPublication("testA", "double");
    printf(" Publication registered\n");

    sub = vfed->registerSubscription("testB");
    printf(" Subscription registered\n");

    /* Register the subscription */

    /* Enter initialization state */
    vfed->enterInitializingMode(); // can throw helicscpp::InvalidStateTransition exception
    printf(" Entered initialization state\n");

    double x = 0.0, /*yprv = 100,*/ xprv = 100;
    helics_time currenttime = 0.0;
    helicscpp::helics_iteration_time currenttimeiter;
    currenttimeiter.status = helics_iteration_result_iterating;
    // int isUpdated;
    double tol = 1E-8;

    pub.publish(x);
    /* Enter execution state */
    vfed->enterExecutingMode(); // can throw helicscpp::InvalidStateTransition exception
    printf(" Entered execution state\n");

    fflush(NULL);
    int helics_iter = 0;
    while (currenttimeiter.status == helics_iteration_result_iterating) {
        //    yprv = y;
        double y = sub.getDouble();
        int newt_conv = 0, max_iter = 10, iter = 0;
        /* Solve the equation using Newton */
        while (!newt_conv && iter < max_iter) {
            /* Function value */
            double f1 = x * x - 2 * x - y + 0.5;

            if (fabs(f1) < tol) {
                newt_conv = 1;
                break;
            }
            iter++;

            /* Jacobian */
            double J1 = 2 * x - 2;

            x = x - f1 / J1;
        }
        ++helics_iter;
        printf("Fed1: iteration %d x=%f, y=%f\n", helics_iter, x, y);

        if ((fabs(x - xprv) > tol) || (helics_iter < 5)) {
            pub.publish(x);
            printf("Fed1: publishing new x\n");
        }
        fflush(NULL);
        currenttimeiter =
            vfed->requestTimeIterative(currenttime, helics_iteration_request_iterate_if_needed);
        xprv = x;
    }

    printf("NLIN1: Federate finalized\n");
    fflush(NULL);
    // Destructor for ValueFederate must be called before close library
    delete vfed;
    while (broker.isConnected()) {
#ifdef _MSC_VER
        Sleep(50);
#else
        usleep(50000); /* Sleep for 50 millisecond */
#endif
    }
    printf("NLIN1: Broker disconnected\n");
    helicsCloseLibrary();
    printf("NLIN1: Library closed\n");
    fflush(NULL);
    return (0);
}
