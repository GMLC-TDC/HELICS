/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
static char help[] = " PI RECEIVER: Simple program to demonstrate the usage of HELICS C Interface.\n\
            This example creates a value federate subscribing to the publication \n\
            registered by PI SENDER.\n\n";

#include <stdio.h>
#include <cpp98/ValueFederate.hpp>
#include <cpp98/helics.hpp>

int main(int /*argc*/,char ** /*argv*/)
{
  std::string    fedinitstring="--federates=1";
  double         deltat=0.01;
  helics_subscription sub;


  std::string helicsversion = helics::getHelicsVersionString();

  printf("PI RECEIVER: Helics version = %s\n",helicsversion.c_str());
  printf("%s",help);

  /* Create Federate Info object that describes the federate properties
   * Set federate name and core type from string
   */
  helics::FederateInfo fi("TestB Federate", "zmq");

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

  /* Enter initialization state */
  vfed->enterInitializationState(); // can throw helics::InvalidStateTransition exception
  printf("PI RECEIVER: Entered initialization state\n");

  /* Enter execution state */
  vfed->enterExecutionState(); // can throw helics::InvalidStateTransition exception
  printf("PI RECEIVER: Entered execution state\n");

  helics_time_t currenttime=0.0;
  double        value = 0.0;

  while(currenttime < 0.20) {
    currenttime = vfed->requestTime(currenttime);

    int isupdated = vfed->isUpdated(sub);
    if(isupdated) {
      /* NOTE: The value sent by sender at time t is received by receiver at time t+deltat */
      value = vfed->getDouble(sub);
      printf("PI RECEIVER: Received value = %4.3f at time %3.2f from PI SENDER\n",value,currenttime);
    }
  }
  vfed->finalize();
  printf("PI RECEIVER: Federate finalized\n");
  // Destructor must be called before close library
  delete vfed;
  helicsCloseLibrary();
  return(0);
}

