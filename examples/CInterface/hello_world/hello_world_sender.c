/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

/* include the HELICS header for value Federates*/
#include <ValueFederate.h>
#include <stdio.h>

int main() {
  helics_federate_info_t fedinfo; /* an information object used to pass information to a federate*/
  const char*    fedinitstring="--federates=1"; /* tell the core to expect only 1 federate*/
  helics_federate vfed; /* object representing the actual federate*/
  helics_publication pub; /* an object representing a publication*/
  helics_time_t currenttime = 0.0; /* the current time of the simulation*/
  helics_status  status;/* the result code from a call to the helics Library*/
  /** create an info structure to define some parameters used in federate creation*/
  fedinfo = helicsFederateInfoCreate();
  /** set the federate name*/
  helicsFederateInfoSetFederateName(fedinfo, "hello_world_sender");
  /** set the core type to use
  can be "test", "ipc", "udp", "tcp", "zmq", "mpi"
  not all are available on all platforms
  and should be set to match the broker and receiver
  zmq is the default*/
  helicsFederateInfoSetCoreTypeFromString(fedinfo,"zmq");
  helicsFederateInfoSetCoreInitString(fedinfo,fedinitstring);
  /** set the period of the federate to 1.0*/
  helicsFederateInfoSetPeriod(fedinfo, 1.0);

  /** create the core using the informational structure*/
  vfed = helicsCreateValueFederate(fedinfo);
  if (vfed == NULL) /*check to make sure the federate was created*/
  {
      return (-2);
  }
  /** register a publication interface on vFed, with a global Name of "hello"
  of a type "string", with no units*/
  pub = helicsFederateRegisterGlobalPublication(vfed, "hello", "string", "");
  if (pub == NULL)
  {
      return (-3);
  }
  /** transition the federate to execution mode
  * the helicsFederateEnterInitializationMode is not necessary if there is nothing to do in the initialization mode
  */
  status=helicsFederateEnterInitializationMode(vfed);
  if (status == helics_error)
  {
      fprintf(stderr, "HELICS failed to enter initialization mode\n");
  }
  status=helicsFederateEnterExecutionMode(vfed);
  if (status == helics_error)
  {
      fprintf(stderr, "HELICS failed to enter initialization mode\n");
  }
  /** the federate is now at time 0*/
  /** publish the the Hello World string this will show up at the next time step of an subscribing federates*/
  helicsPublicationPublishString(pub, "Hello, World");
  /** request that helics grant the federate a time of 1.0*/
  status=helicsFederateRequestTime(vfed, 1.0, &currenttime);
  if (status == helics_error)
  {
      fprintf(stderr, "HELICS request time failed\n");
  }
  /** finalize the federate*/
  helicsFederateFinalize(vfed);
  /** free the memory allocated to the federate*/
  helicsFederateFree(vfed);
  /** close the helics library*/
  helicsCloseLibrary();
  return(0);
}

