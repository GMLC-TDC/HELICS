static char help[] = "Example to demonstrate the usage of HELICS C Interface with two federates.\n\
            This example implements a loose-coupling protocol to exchange values between two federates. \n\
            Here, a ZMQ broker is created and a value federate. The value federate can both.\n\
            publish and subcribe. This federate publishes a value and waits for the value \n\
            published by the other federate. Once the value has arrived, it publishes its next value \n\n";

#include <stdio.h>
#include <ValueFederate_c.h>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(int argc,char **argv)
{
  helics_federate_info_t fedinfo;
  const char*    helicsversion;
  helicsStatus   status;
  helics_broker  broker;
  const char*    initstring="2 --name=mainbroker";
  const char*    fedinitstring="--broker=mainbroker --federates=1";
  int            isconnected;
  double         deltat=0.01;
  helics_value_federate vfed;
  helics_publication pub;
  helics_subscription sub;

  helicsversion = helicsGetVersion();

  printf("PI SENDER: Helics version = %s\n",helicsversion);
  printf("%s",help);

  /* Create broker */
  broker = helicsCreateBroker("zmq","",initstring);

  isconnected = helicsBrokerIsConnected(broker);

  if(isconnected) {
    printf("PI SENDER: Broker created and connected\n");
  }
  
  /* Create Federate Info object that describes the federate properties */
  fedinfo = helicsFederateInfoCreate();
  
  /* Set Federate name */
  status = helicsFederateInfoSetFederateName(fedinfo,"TestA Federate");

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
  printf("PI SENDER: Value federate created\n");

  /* Register the publication */
  pub = helicsRegisterGlobalPublication(vfed,"testA","double","");
  printf("PI SENDER: Publication registered\n");

  /* Subscribe to PI SENDER's publication */
  sub = helicsRegisterSubscription(vfed,"testB","double","");
  printf("PI SENDER: Subscription registered\n");

  /* Register the subscription */

  /* Enter initialization mode */
  status = helicsEnterInitializationMode(vfed);
  printf("PI SENDER: Entered initialization mode\n");

  /* Enter execution mode */
  status = helicsEnterExecutionMode(vfed);
  printf("PI SENDER: Entered execution mode\n");

  /* This federate will be publishing deltat*pi for numsteps steps */
  //double this_time = 0.0;
  double pi = 22.0/7.0,value;
  helics_time_t currenttime=0.0;
  int           numsteps=20,isupdated;

  while(currenttime < 0.2) {
    value = currenttime*pi;

    printf("PI SENDER: Sending value %3.2f*pi = %4.3f at time %3.2f to PI RECEIVER\n",currenttime,value,currenttime);
    status = helicsPublishDouble(pub,value); /* Note: the receiver will get this at currenttime+deltat */

    isupdated = 0;
    while(!isupdated) {
      currenttime = helicsRequestTime(vfed,currenttime);
      isupdated = helicsIsValueUpdated(sub);
    }
     
    /* NOTE: The value sent by sender at time t is received by receiver at time t+deltat */
    status = helicsGetDouble(sub,&value); /* Note: The receiver sent this at currenttime-deltat */
    printf("PI SENDER: Received value = %4.3f at time %3.2f from PI RECEIVER\n",value,currenttime);
  }

  status = helicsFinalize(vfed);
  printf("PI SENDER: Federate finalized\n");


  while(helicsBrokerIsConnected(broker)) {
#ifdef _MSC_VER
	  Sleep(10);
#else
    usleep(1000); /* Sleep for 1 millisecond */
#endif
  }
  printf("PI SENDER: Broker disconnected\n");

  return(0);
}
