/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <helics/cpp98/ValueFederate.hpp>
#include <helics/cpp98/helics.hpp>  // helicsVersionString
#include <iostream>
#include <string>

int main(int /*argc*/, char** /*argv*/)
{
    std::string fedinitstring = "--federates=1";
    double deltat = 0.01;

    std::string helicsversion = helicscpp::getHelicsVersionString();

    if (helicsversion.find("error") == std::string::npos) {
        // this has to do with tests passing on CI builds
        std::cout << "Helics version = " << helicsversion << '\n';
    }

    /* Create Federate Info object that describes the federate properties
     * Set federate name and core type from string
     */
    helicscpp::FederateInfo fedInfo("zmq");

    /* Federate init string */
    fedInfo.setCoreInit(fedinitstring);

    fedInfo.setProperty(HELICS_PROPERTY_TIME_DELTA, deltat);

    fedInfo.setProperty(HELICS_PROPERTY_INT_MAX_ITERATIONS, 100);

    fedInfo.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_WARNING);

    /* Create value federate */
    helicscpp::ValueFederate* vfed = new helicscpp::ValueFederate("TestB Federate", fedInfo);
    std::cout << " Value federate created\n";

    helicscpp::Input sub = vfed->registerSubscription("testA");
    std::cout << " Subscription registered\n";

    /* Register the publication */
    helicscpp::Publication pub = vfed->registerGlobalPublication("testB", HELICS_DATA_TYPE_DOUBLE);
    std::cout << " Publication registered\n";

    /* Enter initialization state */
    vfed->enterInitializingMode();  // can throw helicscpp::InvalidStateTransition exception
    std::cout << " Entered initialization state" << std::endl;
    double y = 1.0;
    double yprv = 100.0;
    pub.publish(y);
    /* Enter execution state */
    vfed->enterExecutingMode();  // can throw helicscpp::InvalidStateTransition exception
    std::cout << " Entered execution state\n";

    HelicsTime currenttime = 0.0;
    helicscpp::HelicsIterationTime currenttimeiter;
    currenttimeiter.status = HELICS_ITERATION_RESULT_ITERATING;

    // int           isupdated;
    double tol = 1E-8;
    int helics_iter = 0;
    while (currenttimeiter.status == HELICS_ITERATION_RESULT_ITERATING) {
        //    xprv = x;
        double x = sub.getDouble();
        ++helics_iter;
        bool newt_conv = false;
        int max_iter = 10;
        int iter = 0;
        /* Solve the equation using Newton */
        while (!newt_conv && iter < max_iter) {
            /* Function value */
            double f2 = x * x + 4 * y * y - 4;

            if (fabs(f2) < tol) {
                newt_conv = true;
            }
            iter++;

            /* Jacobian */
            double J2 = 8 * y;

            y = y - f2 / J2;
        }
        std::cout << "Fed2: iteration " << helics_iter << " x=" << x << ", y=" << y << '\n';

        if ((fabs(y - yprv) > tol) || (helics_iter < 5)) {
            pub.publish(y);
            std::cout << "Fed2: publishing y" << std::endl;
        }
        currenttimeiter =
            vfed->requestTimeIterative(currenttime, HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED);
        yprv = y;
    }

    vfed->finalize();
    std::cout << "NLIN2: Federate finalized" << std::endl;
    // Destructor for ValueFederate must be called before close library
    delete vfed;
    helicsCloseLibrary();
    std::cout << "NLIN2: Library Closed" << std::endl;
    return (0);
}
