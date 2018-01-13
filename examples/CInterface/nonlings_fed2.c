#include <stdio.h>
#include <math.h>
#include <ValueFederate_c.h>
#include <math.h>

int main()
{
  helics_federate_info_t fedinfo;
  const char*    helicsversion;
  helics_status   status;
  const char*    fedinitstring="--broker=mainbroker --federates=1";
  double         deltat=0.01;
  helics_value_federate vfed;
  helics_subscription sub;
  helics_publication  pub;


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

  sub = helicsRegisterSubscription(vfed,"testA","double","");
  printf(" Subscription registered\n");

  /* Register the publication */
  pub = helicsRegisterGlobalPublication(vfed,"testB","double","");
  printf(" Publication registered\n");


  /* Enter initialization mode */
  status = helicsEnterInitializationMode(vfed);
  printf(" Entered initialization mode\n");
  double y = 1.0, x = 0, /*xprv = 100,*/ yprv=100;

  status = helicsPublishDouble(pub, y);
  if (status != helics_ok)
  {
      printf("Error sending publication\n");
  }
  fflush(NULL);
  /* Enter execution mode */
   helicsEnterExecutionMode(vfed);
  printf(" Entered execution mode\n");

  fflush(NULL);
  helics_time_t currenttime=0.0;
  helics_iterative_time currenttimeiter;
  currenttimeiter.status = iterating;

 // int           isupdated;
  double tol=1E-8;
  int helics_iter = 0;
  while (currenttimeiter.status==iterating)
  {

   // xprv = x;
     helicsGetDouble(sub,&x);
    ++helics_iter;
    double f2,J2;
    int    newt_conv = 0, max_iter=10,iter=0;

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
      helicsPublishDouble(pub,y);
      printf("Fed2: publishing y\n");
    }
    fflush(NULL);
    currenttimeiter = helicsRequestTimeIterative(vfed, currenttime, iterate_if_needed);
    yprv = y;
  }

  helicsFederateFinalize(vfed);
  printf("NLIN2: Federate finalized\n");
  fflush(NULL);
  //clean upFederate;
  helicsFreeFederate(vfed);
  helicsCloseLibrary();
  printf("NLIN2: Library Closed\n");
  fflush(NULL);
  return(0);

}
