/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/

#include <ValueFederate.h>
#include <stdio.h>

int main ()
{
    helics_federate_info_t fedinfo; /* an information object used to pass information to a federate*/
    const char *fedinitstring = "--federates=1"; /* tell the core to expect only 1 federate*/
    helics_federate vfed; /* object representing the actual federate*/
    helics_subscription sub; /* an object representing a subscription*/
    helics_time_t currenttime = 0.0; /* the current time of the simulation*/
    helics_status  status;/* the result code from a call to the helics Library*/
    helics_bool_t isUpdated;  /* storage for a check if a value has been updated*/


    /** create an info structure to define some parameters used in federate creation*/
    fedinfo = helicsFederateInfoCreate ();
    /** set the federate name*/
    helicsFederateInfoSetFederateName (fedinfo, "hello_world_receiver");

    /** set the core type to use
    can be "test", "ipc", "udp", "tcp", "zmq", "mpi"
    not all are available on all platforms
    and should be set to match the broker and receiver
    zmq is the default*/
    helicsFederateInfoSetCoreTypeFromString (fedinfo, "zmq");
    helicsFederateInfoSetCoreInitString (fedinfo, fedinitstring);

    /** set the period of the federate to 1.0*/
    helicsFederateInfoSetPeriod(fedinfo, 1.0);

    /** create the core using the informational structure*/
    vfed = helicsCreateValueFederate (fedinfo);
    if (vfed == NULL) /*check to make sure the federate was created*/
    {
        return (-2);
    }

    /** register a subscription interface on vFed, with a Name of "hello"
    of a type "string", with no units*/
    sub = helicsFederateRegisterSubscription (vfed, "hello", "string", NULL);
    if (sub == NULL)
    {
        return (-3);
    }
    /** transition the federate to execution mode
    * the helicsFederateEnterInitializationMode is not necessary if there is nothing to do in the initialization mode
    */
    helicsFederateEnterInitializationMode (vfed);
    helicsFederateEnterExecutionMode (vfed);
    /** request that helics grant the federate a time of 1.0
    the new time will be returned in currentime*/
    status=helicsFederateRequestTime (vfed, 1.0, &currenttime);
    if (status == helics_error)
    {
        fprintf(stderr, "HELICS request time failed\n");
    }
    /** check if the value was updated*/
    isUpdated = helicsSubscriptionIsUpdated (sub);
    if (isUpdated)
    { /* get the value*/
        char value[128] = ""; /**space to store the sent value*/
        helicsSubscriptionGetString(sub, value, 128);
        printf("%s\n", value);
    }
    else
    {
        printf("value was not updated\n");
    }
    /** finalize the federate*/
    helicsFederateFinalize (vfed);
    /** free the memory allocated to the federate*/
    helicsFederateFree (vfed);
    /** close the helics library*/
    helicsCloseLibrary ();
    return (0);
}

