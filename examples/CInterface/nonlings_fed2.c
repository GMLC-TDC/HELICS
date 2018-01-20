/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <stdio.h>
#include <math.h>
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
  double y = 1.0, x = 0, /*xprv = 100,*/ yprv = 100;
  // int           isupdated;
  double tol = 1E-8;
  int helics_iter = 0;

  helicsversion = helicsGetVersion();

  printf(" Helics version = %s\n",helicsversion);

  /* Create Federate Info object that describes the federate properties */
  fedinfo = helicsFederateInfoCreate();
  
  /* Set Federate name */
  helicsFederateInfoSetFederateName(fedinfo,"TestB Federate");

  /* Set core type from string */
  helicsFederateInfoSetCoreTypeFromString(fedinfo,"zmq");

  /* Federate init string */
  helicsFederateInfoSetCoreInitString(fedinfo,fedinitstring);

  helicsFederateInfoSetTimeDelta(fedinfo,deltat);

  helicsFederateInfoSetMaxIterations(fedinfo,100);

   helicsFederateInfoSetLoggingLevel(fedinfo,1);

  /* Create value federate */
  vfed = helicsCreateValueFederate(fedinfo);
  printf(" Value federate created\n");

  sub = helicsFederateRegisterSubscription(vfed,"testA","double","");
  printf(" Subscription registered\n");

  /* Register the publication */
  pub = helicsFederateRegisterGlobalPublication(vfed,"testB","double","");
  printf(" Publication registered\n");


  /* Enter initialization mode */
  status = helicsFederateEnterInitializationMode(vfed);
  printf(" Entered initialization mode\n");
  

  status = helicsPublicationPublishDouble(pub, y);
  if (status != helics_ok)
  {
      printf("Error sending publication\n");
  }
  fflush(NULL);
  /* Enter execution mode */
   helicsFederateEnterExecutionMode(vfed);
  printf(" Entered execution mode\n");

  fflush(NULL);
  helics_time_t currenttime=0.0;
  helics_iteration_status currenttimeiter;
  currenttimeiter = iterating;


  while (currenttimeiter==iterating)
  {
      double f2, J2;
      int    newt_conv = 0, max_iter = 10, iter = 0;
   // xprv = x;
     helicsSubscriptionGetDouble(sub,&x);
    ++helics_iter;
    

    /* Solve the equation using Newton */
    while(!newt_conv && iter < max_iter) {
      /* Function value */
      f2 = x*x + 4*y*y - 4;
      
      if(fabs(f2) < tol) {
	newt_conv = 1;
	break;
      }
      iter++;

      /* Jacobian */
      J2 = 8*y;
      
      y = y - f2/J2;
    }
    printf("Fed2 iteration %d y=%f, x=%f\n",helics_iter,y,x);

   
    if ((fabs(y-yprv)>tol)||(helics_iter<5))
    {
      helicsPublicationPublishDouble(pub,y);
      printf("Fed2: publishing y\n");
    }
    fflush(NULL);
    helicsFederateRequestTimeIterative(vfed, currenttime, iterate_if_needed,&currenttime,&currenttimeiter);
    yprv = y;
  }

  helicsFederateFinalize(vfed);
  printf("NLIN2: Federate finalized\n");
  fflush(NULL);
  //clean upFederate;
  helicsFederateFree(vfed);
  helicsCloseLibrary();
  printf("NLIN2: Library Closed\n");
  fflush(NULL);
  return(0);

}
