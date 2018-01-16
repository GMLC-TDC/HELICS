/*

Copyright (C) 2017-2018, Battelle Memorial Institute
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
#include <cpp98/ValueFederate.hpp>
#include <cpp98/Broker.hpp>
#include <cpp98/helics.hpp> // getHelicsVersionString
#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(int /*argc*/,char ** /*argv*/)
{
  std::string    initstring="2 --name=mainbroker";
  std::string    fedinitstring="--broker=mainbroker --federates=1";
  double         deltat=0.01;
  helics_publication pub;

  std::string helicsversion = helics::getHelicsVersionString();

  printf("PI SENDER: Helics version = %s\n",helicsversion.c_str());
  printf("%s",help);

  /* Create broker */
  helics::Broker broker("zmq","",initstring);

  if(broker.isConnected()) {
    printf("PI SENDER: Broker created and connected\n");
  }

   /* Create Federate Info object that describes the federate properties
    * Sets the federate name and core type from string
    */
  helics::FederateInfo fi("Test sender Federate", "zmq");

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
  printf("PI SENDER: Value federate created\n");

  /* Register the publication */
  pub = vfed->registerGlobalPublication("testA", "double");
  printf("PI SENDER: Publication registered\n");

  /* Enter initialization state */
  vfed->enterInitializationState(); // can throw helics::InvalidStateTransition exception
  printf("PI SENDER: Entered initialization state\n");

  /* Enter execution state */
  vfed->enterExecutionState(); // can throw helics::InvalidStateTransition exception
  printf("PI SENDER: Entered execution state\n");

  /* This federate will be publishing deltat*pi for numsteps steps */
  //double this_time = 0.0;
  double value = 22.0/7.0,val;
  helics_time_t currenttime=0.0;
  int           numsteps=20,i;

  for(i=0; i < numsteps; i++) {
    val = currenttime*value;

    printf("PI SENDER: Sending value %3.2fpi = %4.3f at time %3.2f to PI RECEIVER\n",deltat*i,val,currenttime);
    vfed->publish(pub, val);

    currenttime = vfed->requestTime(currenttime);
  }

  vfed->finalize();
  printf("PI SENDER: Federate finalized\n");

  // destructor must be called before call to helicsCloseLibrary(), or segfault
  delete vfed;

  while(broker.isConnected()) {
#ifdef _MSC_VER
	  Sleep(100);
#else
    usleep(100000); /* Sleep for 100 millisecond */
#endif
  }
  printf("PI SENDER: Broker disconnected\n");
  helicsCloseLibrary();
  return(0);
}
