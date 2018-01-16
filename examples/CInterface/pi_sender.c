/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
static char help[] = " PI SENDER: Simple program to demonstrate the usage of HELICS C Interface.\n\
            This example creates a ZMQ broker and a value federate.\n\
            The value federate creates a global publications and publishes\n\
            t*pi for 20 time-steps with a time-step of 0.01 seconds.\n\n";


#include <stdio.h>
#include <ValueFederate.h>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

int main()
{
  helics_federate_info_t fedinfo;
  const char*    helicsversion;
  //helics_status   status;
  helics_broker  broker;
  const char*    initstring="2 --name=mainbroker";
  const char*    fedinitstring="--broker=mainbroker --federates=1";
  int            isconnected;
  double         deltat=0.01;
  helics_federate vfed;
  helics_publication pub;

  helicsversion = helicsGetVersion();

  printf("PI SENDER: Helics version = %s\n",helicsversion);
  printf("%s",help);

  /* Create broker */
  broker = helicsCreateBroker("zmq","",initstring);

  isconnected = helicsBrokerIsConnected(broker);

  if(isconnected) {
    printf("PI SENDER: Broker created and connected\n");
  }
  
  /* Create Federate Info object that describes the federate properties */
  fedinfo = helicsFederateInfoCreate();
  
  /* Set Federate name */
  helicsFederateInfoSetFederateName(fedinfo,"Test sender Federate");

  /* Set core type from string */
   helicsFederateInfoSetCoreTypeFromString(fedinfo,"zmq");

  /* Federate init string */
  helicsFederateInfoSetCoreInitString(fedinfo,fedinitstring);

  /* Set the message interval (timedelta) for federate. Note that
     HELICS minimum message time interval is 1 ns and by default
     it uses a time delta of 1 second. What is provided to the
     setTimedelta routine is a multiplier for the default timedelta.
  */
  /* Set one second message interval */
  helicsFederateInfoSetTimeDelta(fedinfo,deltat);

  helicsFederateInfoSetLoggingLevel(fedinfo,1);

  /* Create value federate */
  vfed = helicsCreateValueFederate(fedinfo);
  printf("PI SENDER: Value federate created\n");

  /* Register the publication */
  pub = helicsFederateRegisterGlobalPublication(vfed,"testA","double","");
  printf("PI SENDER: Publication registered\n");

  /* Enter initialization mode */
   helicsFederateEnterInitializationMode(vfed);
  printf("PI SENDER: Entered initialization mode\n");

  /* Enter execution mode */
   helicsFederateEnterExecutionMode(vfed);
  printf("PI SENDER: Entered execution mode\n");

  /* This federate will be publishing deltat*pi for numsteps steps */
  //double this_time = 0.0;
  double value = 22.0/7.0,val;
  helics_time_t currenttime=0.0;
  int           numsteps=20,i;

  for(i=0; i < numsteps; i++) {
    val = currenttime*value;

    printf("PI SENDER: Sending value %3.2fpi = %4.3f at time %3.2f to PI RECEIVER\n",deltat*i,val,currenttime);
    helicsPublicationPublishDouble(pub,val);

    helicsFederateRequestTime(vfed,currenttime, &currenttime);
  }

  helicsFederateFinalize(vfed);
  printf("PI SENDER: Federate finalized\n");

  helicsFederateFree(vfed);
  while(helicsBrokerIsConnected(broker)) {
#ifdef _MSC_VER
	  Sleep(100);
#else
    usleep(100000); /* Sleep for 100 millisecond */
#endif
  }
  printf("PI SENDER: Broker disconnected\n");
  helicsBrokerFree(broker);
  helicsCloseLibrary();
  return(0);
}
