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
#include <cpp98/ValueFederate.hpp>
#include <cpp98/helics.hpp> // helicsVersionString
#include "core/helicsVersion.hpp"
#include <math.h>

int main(int /*argc*/,char ** /*argv*/)
{
  std::string    fedinitstring="--federates=1";
  double         deltat=0.01;
  helics_subscription sub;
  helics_publication  pub;


  std::string helicsversion = helics::helicsVersionString();

  printf("PI RECEIVER: Helics version = %s\n",helicsversion.c_str());
  printf("%s",help);

  /* Create Federate Info object that describes the federate properties
   * Set federate name and core type from string
   */
  helics::FederateInfo fi("Test receiver Federate", "zmq");

  /* Federate init string */
  fi.setCoreInitString(fedinitstring);

  /* Set the message interval (timedelta) for federate. Note that
     HELICS minimum message time interval is 1 ns and by default
     it uses a time delta of 1 second. What is provided to the
     setTimedelta routine is a multiplier for the default timedelta.
  */
  /* Set one second message interval */
  fi.setTimeDelta(deltat);
  fi.setLoggingLevel(1);

  /* Create value federate */
  helics::ValueFederate* vfed = new helics::ValueFederate(fi);
  printf("PI RECEIVER: Value federate created\n");

  /* Subscribe to PI SENDER's publication */
  sub = vfed->registerSubscription("testA","double");
  printf("PI RECEIVER: Subscription registered\n");

  /* Register the publication */
  pub = vfed->registerGlobalPublication("testB","double");
  printf("PI RECEIVER: Publication registered\n");

  fflush(NULL);
  /* Enter initialization state */
  vfed->enterInitializationState(); // can throw helics::InvalidStateTransition exception
  printf("PI RECEIVER: Entered initialization state\n");

  /* Enter execution state */
  vfed->enterExecutionState(); // can throw helics::InvalidStateTransition exception
  printf("PI RECEIVER: Entered execution state\n");

  helics_time_t currenttime=0.0;
  double        value = 0.0;
  int isupdated=0; 
  double pi = 22.0/7.0;

  while(currenttime < 0.2) {
    
    isupdated = 0;
    while(!isupdated) {
      currenttime = vfed->requestTime(currenttime);
      isupdated = vfed->isUpdated(sub);
	  if (currenttime > 0.21)
	  {
		  break;
	  }
    }
    value = vfed->getDouble(sub); /* Note: The sender sent this value at currenttime-deltat */
    printf("PI RECEIVER: Received value = %4.3f at time %3.2f from PI SENDER\n",value,currenttime);

    value = currenttime*pi;

    printf("PI RECEIVER: Sending value %3.2f*pi = %4.3f at time %3.2f to PI SENDER\n",currenttime,value,currenttime);
    vfed->publish(pub,value); /* Note: The sender will receive this at currenttime+deltat */
  }
  vfed->finalize();
  printf("PI RECEIVER: Federate finalized\n");
  fflush(NULL);
  // Destructor must be called for ValueFederate before close library
  delete vfed;
  helicsCloseLibrary();
  printf("PI RECEIVER: Library Closed\n");
  fflush(NULL);
  return(0);
}
