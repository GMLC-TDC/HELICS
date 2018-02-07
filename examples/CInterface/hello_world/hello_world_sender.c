/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include <stdio.h>
#include <ValueFederate.h>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

int main() {
  helics_federate_info_t fedinfo;
  const char*    fedinitstring="--federates=1";
  helics_federate vfed;
  helics_publication pub;
  helics_time_t currenttime = 0.0;

  fedinfo = helicsFederateInfoCreate();
  helicsFederateInfoSetFederateName(fedinfo, "Test sender Federate");
  helicsFederateInfoSetCoreTypeFromString(fedinfo,"zmq");
  helicsFederateInfoSetCoreInitString(fedinfo,fedinitstring);

  vfed = helicsCreateValueFederate(fedinfo);

  pub = helicsFederateRegisterGlobalPublication(vfed, "testA", "string", "");

  helicsFederateEnterInitializationMode(vfed);
  helicsFederateEnterExecutionMode(vfed);

  helicsPublicationPublishString(pub, "Hello, World");
  helicsFederateRequestTime(vfed, currenttime, &currenttime);

  helicsFederateFinalize(vfed);
  helicsFederateFree(vfed);
  helicsCloseLibrary();
  return(0);
}
