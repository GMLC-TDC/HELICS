/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <MessageFederate.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

/*
static const helics::ArgDescriptors InfoArgs{
    {"startbroker","start a broker with the specified arguments"},
    {"target,t", "name of the target federate"},
    { "messagetarget", "name of the target federate, same as target" },
    {"endpoint,e", "name of the target endpoint"},
    {"source,s", "name of the source endpoint"}
    //name is captured in the argument processor for federateInfo
};
*/
static const char defTarget[] = "fed";
static const char defTargetEndpoint[] = "endpoint";
static const char defSourceEndpoint[] = "endpoint";

int main (int argc, char *argv[])
{
    helics_federate_info_t fedinfo = helicsFederateInfoCreate();
	const char *target = defTarget;
    const char *endpoint = defTargetEndpoint;
    const char *source = defSourceEndpoint;
    char *targetEndpoint = NULL;
    int ii;
    helics_federate mFed = NULL;
    helics_endpoint ept = NULL;
    char str[255];
    char message[1024];
    helics_time_t newTime;
    for (ii = 1; ii < argc; ++ii)
    {
        
        if (strcmp(argv[ii], "target")==0)
        {
            target=argv[ii + 1];
            ++ii;
        }
        else if (strcmp(argv[ii], "endpoint")==0)
        {
            endpoint = argv[ii + 1];
            ++ii;
        }
        else if (strcmp(argv[ii], "source") == 0)
        {
            source = argv[ii + 1];
            ++ii;
        }
        else if (strcmp(argv[ii], "help") == 0)
        {
            //print something
            return 0;
        }
      
    }
	
    helicsFederateInfoSetFederateName(fedinfo, "fed");
    helicsFederateInfoLoadFromArgs(fedinfo, argc, argv);
    
    mFed = helicsCreateMessageFederate(fedinfo);

    targetEndpoint = (char *)malloc(strlen(target) + 2 + strlen(endpoint));
    strcpy(targetEndpoint, target);
    strcat(targetEndpoint, "/");
    strcat(targetEndpoint, endpoint);

    helicsFederateGetName(mFed, str, 255);
    printf("registering endpoint %s for %s\n", targetEndpoint, str);
    //this line actually creates an endpoint
    ept = helicsFederateRegisterEndpoint(mFed, targetEndpoint, "");

    printf("entering init Mode\n");
    helicsFederateEnterInitializationMode(mFed);
    printf("entered init Mode\n");
    helicsFederateEnterExecutionMode(mFed);
    printf("entered execution Mode\n");
    for (int i=1; i<10; ++i) {
        snprintf(message,1024, "message sent from %s to %s at time %d", str, targetEndpoint, i);
        helicsEndpointSendMessageRaw(ept, targetEndpoint, message, (int)(strlen(message)));
		
        printf(" %s \n", message);
        helicsFederateRequestTime(mFed, (helics_time_t)i, &newTime);

        printf("granted time %f\n", newTime);
		while (helicsEndpointHasMessage(ept)==helics_true)
		{
			message_t nmessage = helicsEndpointGetMessage(ept);
            printf("received message from %s at %f ::%s\n", nmessage.source, nmessage.time, nmessage.data);
		}

    }
    helicsFederateFinalize(mFed);
   
    return 0;
}

