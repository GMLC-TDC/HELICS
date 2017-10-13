static char help[] = "Example to demonstrate the usage of HELICS C Interface with two federates.\n\
            This example implements a loose-coupling protocol to exchange values between two federates. \n\
            Here, a value federate, that can both publish and subcribe is created.\n\
            This federate can only publish a value once it receives value from the other federate.\n\n";

#include <stdio.h>
#include <ValueFederate_c.h>
#include <math.h>

int main(int argc,char **argv)
{
  helics_federate_info_t fedinfo;
  const char*    helicsversion;
  helicsStatus   status;
  const char*    fedinitstring="--broker=mainbroker --federates=1";
  double         deltat=0.01;
  helics_value_federate vfed;
  helics_subscription sub;
  helics_publication  pub;


  helicsversion = helicsGetVersion();

  printf("PI RECEIVER: Helics version = %s\n",helicsversion);
  printf("%s",help);

  /* Create Federate Info object that describes the federate properties */
  fedinfo = helicsFederateInfoCreate();
  
  /* Set Federate name */
  status = helicsFederateInfoSetFederateName(fedinfo,"TestB Federate");

  /* Set core type from string */
  status = helicsFederateInfoSetCoreTypeFromString(fedinfo,"zmq");

  /* Federate init string */
  status = helicsFederateInfoSetCoreInitString(fedinfo,fedinitstring);

  /* Set the message interval (timedelta) for federate. Note that
     HELICS minimum message time interval is 1 ns and by default
     it uses a time delta of 1 second. What is provided to the
     setTimedelta routine is a multiplier for the default timedelta.
  */
  /* Set one second message interval */
  status = helicsFederateInfoSetTimeDelta(fedinfo,deltat);

  status = helicsFederateInfoSetLoggingLevel(fedinfo,1);

  /* Create value federate */
  vfed = helicsCreateValueFederate(fedinfo);
  printf("PI RECEIVER: Value federate created\n");

  /* Subscribe to PI SENDER's publication */
  sub = helicsRegisterSubscription(vfed,"testA","double","");
  printf("PI RECEIVER: Subscription registered\n");

  /* Register the publication */
  pub = helicsRegisterGlobalPublication(vfed,"testB","double","");
  printf("PI RECEIVER: Publication registered\n");


  /* Enter initialization mode */
  status = helicsEnterInitializationMode(vfed);
  printf("PI RECEIVER: Entered initialization mode\n");

  /* Enter execution mode */
  status = helicsEnterExecutionMode(vfed);
  printf("PI RECEIVER: Entered execution mode\n");

  helics_time_t currenttime=0.0;
  double        value = 0.0;
  int isupdated=0; 
  double pi = 22.0/7.0;

  while(currenttime < 0.2) {
    
    isupdated = 0;
    while(!isupdated) {
      currenttime = helicsRequestTime(vfed,currenttime);
      isupdated = helicsIsValueUpdated(sub);
    }
    status = helicsGetDouble(sub,&value); /* Note: The sender sent this value at currenttime-deltat */
    printf("PI RECEIVER: Received value = %4.3f at time %3.2f from PI SENDER\n",value,currenttime);

    value = currenttime*pi;

    printf("PI RECEIVER: Sending value %3.2f*pi = %4.3f at time %3.2f to PI SENDER\n",currenttime,value,currenttime);
    status = helicsPublishDouble(pub,value); /* Note: The sender will receive this at currenttime+deltat */
  }
  status = helicsFinalize(vfed);
  printf("PI RECEIVER: Federate finalized\n");

  return(0);
}
