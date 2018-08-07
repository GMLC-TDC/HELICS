/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <cpp98/ValueFederate.hpp>
#include <cpp98/Broker.hpp>
#include <cpp98/helics.hpp> // helicsVersionString
#include <math.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

int main (int /*argc*/, char ** /*argv*/)
{
    std::string initstring = "2 --name=mainbroker";
    std::string fedinitstring = "--broker=mainbroker --federates=1";
    double deltat = 0.01;
    helics98::Publication pub;
    helics98::Input sub;

    std::string helicsversion = helics98::getHelicsVersionString();

    printf (" Helics version = %s\n", helicsversion.c_str());

    /* Create broker */
    helics98::Broker broker("zmq", "", initstring);

    if (broker.isConnected())
    {
        printf (" Broker created and connected\n");
    }

    /* Create Federate Info object that describes the federate properties
     * Set federate name and core type from string
     */
    helics98::FederateInfo fi ("zmq");

    /* Federate init string */
    fi.setCoreInitString (fedinitstring);
    fi.setTimeProperty (TIME_DELTA_PROPERTY, deltat);
    fi.setIntegerProperty (MAX_ITERATIONS_PROPERTY, 100);

    //fi.setLoggingLevel(5);

    /* Create value federate */
    helics98::ValueFederate* vfed = new helics98::ValueFederate ("TestA Federate", fi);
    printf (" Value federate created\n");

    /* Register the publication */
    pub = vfed->registerGlobalTypePublication ("testA", "double");
    printf (" Publication registered\n");

    sub = vfed->registerSubscription ("testB");
    printf (" Subscription registered\n");

    /* Register the subscription */

    /* Enter initialization state */
    vfed->enterInitializingMode (); // can throw helics98::InvalidStateTransition exception
    printf (" Entered initialization state\n");

    double x = 0.0, /*yprv = 100,*/ xprv=100;
    helics_time_t currenttime = 0.0;
    helics98::helics_iteration_time currenttimeiter;
    currenttimeiter.status = iterating;
   // int isUpdated;
    double tol = 1E-8;

    pub.publish (x);
    /* Enter execution state */
    vfed->enterExecutingMode (); // can throw helics98::InvalidStateTransition exception
    printf (" Entered execution state\n");

    fflush (NULL);
    int helics_iter = 0;
    while (currenttimeiter.status == iterating)
    {
    //    yprv = y;
        double y = sub.getDouble ();
        int newt_conv = 0, max_iter = 10, iter = 0;
        /* Solve the equation using Newton */
        while (!newt_conv && iter < max_iter)
        {
            /* Function value */
            double f1 = x * x - 2 * x - y + 0.5;

            if (fabs (f1) < tol)
            {
                newt_conv = 1;
                break;
            }
            iter++;

            /* Jacobian */
            double J1 = 2 * x - 2;

            x = x - f1 / J1;
        }
        ++helics_iter;
        printf("Fed1: iteration %d x=%f, y=%f\n",helics_iter, x, y);

        if ((fabs(x-xprv)>tol)||(helics_iter<5))
        {
            pub.publish (x);
            printf("Fed1: publishing new x\n");
        }
        fflush(NULL);
        currenttimeiter = vfed->requestTimeIterative(currenttime, iterate_if_needed);
        xprv = x;
    }

    printf ("NLIN1: Federate finalized\n");
    fflush (NULL);
    // Destructor for ValueFederate must be called before close library
    delete vfed;
    while (broker.isConnected())
    {
#ifdef _MSC_VER
        Sleep (50);
#else
        usleep (50000); /* Sleep for 50 millisecond */
#endif
    }
    printf ("NLIN1: Broker disconnected\n");
    helicsCloseLibrary ();
    printf ("NLIN1: Library closed\n");
    fflush (NULL);
    return (0);
}

