
/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include <ValueFederate.h>
#include <stdio.h>

int main ()
{
    helics_federate_info_t fedinfo;
    const char *fedinitstring = "--federates=1";
    helics_federate vfed;
    helics_subscription sub;
    helics_time_t currenttime = 0.0;
    int isupdated = 0;
    char value[128] = "";

    fedinfo = helicsFederateInfoCreate ();
    helicsFederateInfoSetFederateName (fedinfo, "TestB Federate");
    helicsFederateInfoSetCoreTypeFromString (fedinfo, "zmq");
    helicsFederateInfoSetCoreInitString (fedinfo, fedinitstring);

    vfed = helicsCreateValueFederate (fedinfo);
    sub = helicsFederateRegisterSubscription (vfed, "testA", "string", "");

    helicsFederateEnterInitializationMode (vfed);
    helicsFederateEnterExecutionMode (vfed);

    helicsFederateRequestTime (vfed, currenttime, &currenttime);

    isupdated = helicsSubscriptionIsUpdated (sub);
    helicsSubscriptionGetString (sub, value, 128);
    printf("%s\n", value);

    helicsFederateFinalize (vfed);
    helicsFederateFree (vfed);
    helicsCloseLibrary ();
    return (0);
}
