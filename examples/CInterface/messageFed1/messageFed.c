/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <ValueFederate.h>
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
    helics_federate_info_t fedinfo;
	char *target = defTarget;
    char *endpoint = defTargetEndpoint;
    char *source = defSourceEndpoint;
    char *targetEndpoint = NULL;
    int ii;
    helics_federate mFed = NULL;
    helics_broker brk = NULL;
    helics_endpoint ept = NULL;
    int startbroker = 0;
    char str[255];

    for (ii = 1; ii < argc; ++ii)
    {
        
        if (strcmp(argv[ii], "target")==0)
        {
            target=argv[ii + 1];
            ++ii;
        }
        else if (strcmp(argv[ii], "endpoint")==0)
        {
            target = argv[ii + 1];
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

    auto name = mFed->getName();
	std::cout << " registering endpoint '" << myendpoint << "' for " << name<<'\n';

    //this line actually creates an endpoint
    auto id = mFed->registerEndpoint(myendpoint);


    std::cout << "entering init State\n";
    helicsFederateEnterInitializionMode(mFed);
    std::cout << "entered init State\n";
    mFed->enterExecutionState ();
    helicsFederateEnterExecutionMode(mFed);
    std::cout << "entered exec State\n";
    for (int i=1; i<10; ++i) {
		std::string message = "message sent from "+name+" to "+target+" at time " + std::to_string(i);
		mFed->sendMessage(id, target, message.data(), message.size());
        std::cout << message << std::endl;
        auto newTime = mFed->requestTime (i);
		std::cout << "processed time " << static_cast<double> (newTime) << "\n";
		while (mFed->hasMessage(id))
		{
			auto nmessage = mFed->getMessage(id);
			std::cout << "received message from " << nmessage->source << " at " << static_cast<double>(nmessage->time) << " ::" << nmessage->data.to_string() << '\n';
		}

    }
    helicsFederateFinalize(mFed);
   
    return 0;
}

