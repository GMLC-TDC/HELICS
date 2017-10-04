static char help[] = " PI RECEIVER: Simple program to demonstrate the usage of HELICS C Interface.\n\
            This example creates a value federate subscribing to the publication \n\
            registered by PI SENDER.\n\n";

#include <stdio.h>
#include <ValueFederate_c.h>

int main(int argc,char **argv)
{
  helics_federate_info_t fedinfo;
  const char*    helicsversion;
  helicsStatus   status;
  const char*    fedinitstring="--broker=mainbroker --federates=1";
  double         deltat=0.01;
  helics_value_federate vfed;
  helics_subscription sub;


  helicsversion = helicsGetVersion();

  printf("PI RECEIVER: Helics version = %s\n",helicsversion);
  printf("%s",help);

  /* Create Federate Info object that describes the federate properties */
  fedinfo = createFederateInfoObject();
  
  /* Set Federate name */
  status = FederateInfoSetFederateName(fedinfo,"TestB Federate");

  /* Set core type from string */
  status = FederateInfoSetCoreTypeFromString(fedinfo,"zmq");

  /* Federate init string */
  status = FederateInfoSetCoreInitString(fedinfo,fedinitstring);

  /* Set the message interval (timedelta) for federate. Note that
     HELICS minimum message time interval is 1 ns and by default
     it uses a time delta of 1 second. What is provided to the
     setTimedelta routine is a multiplier for the default timedelta.
  */
  /* Set one second message interval */
  status = FederateInfoSetTimeDelta(fedinfo,helicsTimeFromDouble(deltat));

  status = FederateInfoSetLoggingLevel(fedinfo,1);

  /* Create value federate */
  vfed = helicsCreateValueFederate(fedinfo);
  printf("PI RECEIVER: Value federate created\n");

  /* Subscribe to PI SENDER's publication */
  sub = helicsRegisterSubscription(vfed,"testA","double","");
  printf("PI RECEIVER: Subscription registered\n");

  status = helicsEnterExecutionMode(vfed);
  printf("PI RECEIVER: Entering execution mode\n");

  helics_time_t currenttimeobj,prevtimeobj;
  double        value=0.0,currenttime=0.0;

  prevtimeobj = helicsRequestTime(vfed,helicsTimeFromDouble(0.19));
  
  while(currenttime <= 0.19) {

    currenttimeobj = helicsRequestTime(vfed,helicsTimeFromDouble(0.19));
    currenttime = doubleFromHelicsTime(currenttimeobj);

    /* HELICSVALUEFEDERATEISUPDATED IS NOT WORKING CORRECTLY.
       Trying to check if the subscription is updated with new values
       by checking the current and previous time objects. This is a hack
       and should be removed once helicsValueFederateisUpdated is working
       correctly
    */
    /*   int isupdated; */
    /*    isupdated = helicsValueFederateisUpdated(vfed,sub); */
    /*    if(isupdated) { */
    if(currenttimeobj > prevtimeobj) {
      status = helicsGetDouble(sub,&value);
      printf("PI RECEIVER: Received value = %4.3f at time %3.2f from PI SENDER\n",value,currenttime);
      /*    } */
    }
    prevtimeobj = currenttimeobj;
  }
  status = helicsFinalize(vfed);
  printf("PI RECEIVER: Federate finalized\n");

  return(0);
}
