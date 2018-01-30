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

/* This solves the system being simulated by this simulator. It takes in the coupling variable
   x, convergence tolerance tol, and returns the state variable yout and the converged status converged
*/
void run_sim2(double x,double tol,double *yout,int *converged)
{
  double f2,J2,y=*yout;
  int    newt_conv = 0, max_iter=10,iter=0;

  /* Solve the equation using Newton */
  while(!newt_conv && iter < max_iter) {
    /* Function value */
    f2 = x*x + 4*y*y - 4;

    /* Convergence check */
    if(fabs(f2) < tol) {
      newt_conv = 1;
      break;
    }
    iter++;

    /* Jacobian */
    J2 = 8*y;

    /* Update */
    y = y - f2/J2;
  }
  *yout = y;
  *converged = newt_conv;
}

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
  int converged;

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
  double y = 1.0, x = 0, /*xprv = 100,*/ yprv=100;

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

 // int           isupdated;
  double tol=1E-8;
  int helics_iter = 0;
  while (currenttimeiter==iterating)
  {

   // xprv = x;
     helicsSubscriptionGetDouble(sub,&x);
     /* Solve the system of equations for this federate */
     run_sim2(x,tol,&y,&converged);
    ++helics_iter;
    printf("Fed2 Current time %4.3f iteration %d x=%f, y=%f\n",currenttime, helics_iter,x,y);

   
    if ((fabs(y-yprv)>tol))
    {
      helicsPublicationPublishDouble(pub,y);
      printf("Fed2: publishing new y\n");
    } else {
      printf("Fed2: converged\n");
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
