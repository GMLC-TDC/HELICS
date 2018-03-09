/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
static char help[] = "Example to demonstrate the usage of HELICS C Interface with two federates.\n\
            This example implements a loose-coupling protocol to exchange values between two federates. \n\
            Here, a value federate, that can both publish and subcribe is created.\n\
            This federate can only publish a value once it receives value from the other federate.\n\n";

#include <stdio.h>
#include <ValueFederate.h>
#include <math.h>

int main()
{
  helics_federate_info_t fedinfo;
  const char*    helicsversion;
  helics_status   status;
  const char*    fedinitstring="--federates=1";
  double         deltat=0.01;
  helics_federate vfed;
  helics_subscription sub;
  helics_publication  pub;
  helics_time_t currenttime = 0.0;
  double        value = 0.0;
  double pi = 22.0 / 7.0;

  helicsversion = helicsGetVersion();

  printf("PI RECEIVER: Helics version = %s\n",helicsversion);
  printf("%s",help);

  /* Create Federate Info object that describes the federate properties */
  fedinfo = helicsFederateInfoCreate();

  /* Set Federate name */
  helicsFederateInfoSetFederateName(fedinfo,"Test receiver Federate");

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
  printf("PI RECEIVER: Value federate created\n");

  /* Subscribe to PI SENDER's publication */
  sub = helicsFederateRegisterSubscription(vfed,"testA","double","");
  printf("PI RECEIVER: Subscription registered\n");

  /* Register the publication */
  pub = helicsFederateRegisterGlobalPublication(vfed,"testB","double","");
  printf("PI RECEIVER: Publication registered\n");

  fflush(NULL);
  /* Enter initialization mode */
  if ((status = helicsFederateEnterInitializationMode(vfed)) == helics_ok)
  {
      printf("PI RECEIVER: Entered initialization mode\n");
  }
  else
  {
      return (-3);
  }


  /* Enter execution mode */
  if ((status = helicsFederateEnterExecutionMode(vfed)) == helics_ok)
  {
      printf("PI RECEIVER: Entered execution mode\n");
  }


  while(currenttime < 0.2) {

     int isupdated = 0;
    while(!isupdated) {
      helicsFederateRequestTime(vfed,currenttime, &currenttime);
      isupdated = helicsSubscriptionIsUpdated(sub);
	  if (currenttime > 0.21)
	  {
		  break;
	  }
    }
    helicsSubscriptionGetDouble(sub,&value); /* Note: The sender sent this value at currenttime-deltat */
    printf("PI RECEIVER: Received value = %4.3f at time %3.2f from PI SENDER\n",value,currenttime);

    value = currenttime*pi;

    printf("PI RECEIVER: Sending value %3.2f*pi = %4.3f at time %3.2f to PI SENDER\n",currenttime,value,currenttime);
    helicsPublicationPublishDouble(pub,value); /* Note: The sender will receive this at currenttime+deltat */
  }
  helicsFederateFinalize(vfed);
  printf("PI RECEIVER: Federate finalized\n");
  fflush(NULL);
  /*clean upFederate*/
  helicsFederateFree(vfed);
  helicsCloseLibrary();
  printf("PI RECEIVER: Library Closed\n");
  fflush(NULL);
  return(0);
}

