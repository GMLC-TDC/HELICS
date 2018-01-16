/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <ValueFederate.h>
#include <math.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

int main ()
{
    helics_federate_info_t fedinfo;
    const char *helicsversion;
    helics_status status;
    helics_broker broker;
    const char *initstring = "2 --name=mainbroker";
    const char *fedinitstring = "--broker=mainbroker --federates=1";
    int isconnected;
    double deltat = 0.01;
    helics_federate vfed;
    helics_publication pub;
    helics_subscription sub;

    helicsversion = helicsGetVersion ();

    printf (" Helics version = %s\n", helicsversion);

    /* Create broker */
    broker = helicsCreateBroker ("zmq", "", initstring);

    isconnected = helicsBrokerIsConnected (broker);

    if (isconnected)
    {
        printf (" Broker created and connected\n");
    }

    /* Create Federate Info object that describes the federate properties */
    fedinfo = helicsFederateInfoCreate ();

    /* Set Federate name */
     helicsFederateInfoSetFederateName (fedinfo, "TestA Federate");

    /* Set core type from string */
     helicsFederateInfoSetCoreTypeFromString (fedinfo, "zmq");

    /* Federate init string */
     helicsFederateInfoSetCoreInitString (fedinfo, fedinitstring);

     helicsFederateInfoSetTimeDelta (fedinfo, deltat);

    helicsFederateInfoSetMaxIterations (fedinfo, 100);

    //status = helicsFederateInfoSetLoggingLevel (fedinfo, 5);

    /* Create value federate */
    vfed = helicsCreateValueFederate (fedinfo);
    printf (" Value federate created\n");

    /* Register the publication */
    pub = helicsFederateRegisterGlobalPublication (vfed, "testA", "double", "");
    printf (" Publication registered\n");

    sub = helicsFederateRegisterSubscription (vfed, "testB", "double", "");
    printf (" Subscription registered\n");

    /* Register the subscription */

    /* Enter initialization mode */
    status = helicsFederateEnterInitializationMode (vfed);
    if (status == helics_ok)
    {
        printf(" Entered initialization mode\n");
    }
    else
    {
        return (-3);
    }

    double x = 0.0, y = 0.0, /*yprv = 100,*/ xprv=100;
    helics_time_t currenttime = 0.0;
    helics_iteration_status currenttimeiter;
    currenttimeiter = iterating;
   // int isupdated;
    double tol = 1E-8;

    helicsPublicationPublishDouble (pub, x);
    /* Enter execution mode */
    status = helicsFederateEnterExecutionMode (vfed);
    if (status == helics_ok)
    {
        printf(" Entered execution mode\n");
    }
    else
    {
        return (-3);
    }
    

    fflush (NULL);
    int helics_iter = 0;
    while (currenttimeiter == iterating)
    {
       // yprv = y;
         helicsSubscriptionGetDouble (sub, &y);
        double f1, J1;
        int newt_conv = 0, max_iter = 10, iter = 0;
        /* Solve the equation using Newton */
        while (!newt_conv && iter < max_iter)
        {
            /* Function value */
            f1 = x * x - 2 * x - y + 0.5;

            if (fabs (f1) < tol)
            {
                newt_conv = 1;
                break;
            }
            iter++;

            /* Jacobian */
            J1 = 2 * x - 2;

            x = x - f1 / J1;
        }
        ++helics_iter;
        printf("Fed1: iteration %d x=%f, y=%f\n",helics_iter, x, y);
        
        if ((fabs(x-xprv)>tol)||(helics_iter<5))
        {
            helicsPublicationPublishDouble (pub, x);
            printf("Fed1: publishing new x\n");
        }
        fflush(NULL);
         helicsFederateRequestTimeIterative(vfed, currenttime, iterate_if_needed,&currenttime,&currenttimeiter);
        xprv = x;
    }

    printf ("NLIN1: Federate finalized\n");
    fflush (NULL);
    helicsFederateFree (vfed);
    while (helicsBrokerIsConnected (broker))
    {
#ifdef _MSC_VER
        Sleep (50);
#else
        usleep (50000); /* Sleep for 50 millisecond */
#endif
    }
    helicsBrokerFree(broker);
    printf ("NLIN1: Broker disconnected\n");
    helicsCloseLibrary ();
    printf ("NLIN1: Library closed\n");
    fflush (NULL);
    return (0);
}
