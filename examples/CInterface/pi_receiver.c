static char help[] = " PI RECEIVER: Simple program to demonstrate the usage of HELICS C Interface.\n\
            This example creates a value federate subscribing to the publication \n\
            registered by PI SENDER.\n\n";

#include <stdio.h>
#include <ValueFederate_c.h>

int main()
{
  helics_federate_info_t fedinfo;
  const char*    helicsversion;
  helics_status   status;
  const char*    fedinitstring="--broker=mainbroker --federates=1";
  double         deltat=0.01;
  helics_value_federate vfed;
  helics_subscription sub;


  helicsversion = helicsGetVersion();

  printf("PI RECEIVER: Helics version = %s\n",helicsversion);
  printf("%s",help);

  /* Create Federate Info object that describes the federate properties */
  fedinfo = helicsFederateInfoCreate();
  
  /* Set Federate name */
  helicsFederateInfoSetFederateName(fedinfo,"TestB Federate");

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
  sub = helicsRegisterSubscription(vfed,"testA","double","");
  printf("PI RECEIVER: Subscription registered\n");

  /* Enter initialization mode */
  if ((status = helicsEnterInitializationMode(vfed)) == helics_ok)
  {
      printf("PI RECEIVER: Entered initialization mode\n");
  }
  else
  {
      return (-3);
  }
  

  /* Enter execution mode */
  if ((status = status = helicsEnterExecutionMode(vfed)) == helics_ok)
  {
      printf("PI RECEIVER: Entered execution mode\n");
  }
  

  helics_time_t currenttime=0.0;
  double        value = 0.0;
  int isupdated=0; 

  while(currenttime < 0.20) {
    currenttime = helicsRequestTime(vfed,currenttime);

    isupdated = helicsIsValueUpdated(sub);
    if(isupdated) {
      /* NOTE: The value sent by sender at time t is received by receiver at time t+deltat */
       helicsGetDouble(sub,&value);
      printf("PI RECEIVER: Received value = %4.3f at time %3.2f from PI SENDER\n",value,currenttime);
    }
  }
  helicsFederateFinalize(vfed);
  printf("PI RECEIVER: Federate finalized\n");
  helicsFreeFederate(vfed);
  helicsCloseLibrary();
  return(0);
}
