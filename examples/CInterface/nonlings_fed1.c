#include <ValueFederate_c.h>
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
    helicsStatus status;
    helics_broker broker;
    const char *initstring = "2 --name=mainbroker";
    const char *fedinitstring = "--broker=mainbroker --federates=1";
    int isconnected;
    double deltat = 0.01;
    helics_value_federate vfed;
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
    status = helicsFederateInfoSetFederateName (fedinfo, "TestA Federate");

    /* Set core type from string */
    status = helicsFederateInfoSetCoreTypeFromString (fedinfo, "zmq");

    /* Federate init string */
    status = helicsFederateInfoSetCoreInitString (fedinfo, fedinitstring);

    status = helicsFederateInfoSetTimeDelta (fedinfo, deltat);

    status = helicsFederateInfoSetMaxIterations (fedinfo, 100);

    //status = helicsFederateInfoSetLoggingLevel (fedinfo, 5);

    /* Create value federate */
    vfed = helicsCreateValueFederate (fedinfo);
    printf (" Value federate created\n");

    /* Register the publication */
    pub = helicsRegisterGlobalPublication (vfed, "testA", "double", "");
    printf (" Publication registered\n");

    sub = helicsRegisterSubscription (vfed, "testB", "double", "");
    printf (" Subscription registered\n");

    /* Register the subscription */

    /* Enter initialization mode */
    status = helicsEnterInitializationMode (vfed);
    printf (" Entered initialization mode\n");

    double x = 0.0, y = 0.0, yprv = 100, xprv=100;
    helics_time_t currenttime = 0.0;
    helics_iterative_time currenttimeiter;
    currenttimeiter.status = iterating;
    int isupdated;
    double tol = 1E-8;

    status = helicsPublishDouble (pub, x);
    /* Enter execution mode */
    status = helicsEnterExecutionMode (vfed);
    printf (" Entered execution mode\n");

    fflush (NULL);
    int helics_iter = 0;
    while (currenttimeiter.status == iterating)
    {
        yprv = y;
        status = helicsGetDouble (sub, &y);
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
            status = helicsPublishDouble (pub, x);
            printf("Fed1: publishing new x\n");
        }
        fflush(NULL);
        currenttimeiter = helicsRequestTimeIterative(vfed, currenttime, iterate_if_needed);
        xprv = x;
    }

    printf ("NLIN1: Federate finalized\n");
    fflush (NULL);
    helicsFreeFederate (vfed);
    while (helicsBrokerIsConnected (broker))
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
