static char help[] = " PI SENDER: Simple program to demonstrate the usage of HELICS C Interface.\n\
            This example creates a ZMQ broker and a value federate.\n\
            The value federate creates a global pulications and publishes\n\
            t*pi for 20 time-steps with a time-step of 0.01 seconds.\n\n";


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

  helicsversion = helicsGetVersion();

  printf("PI SENDER: Helics version = %s\n",helicsversion);
  printf("%s",help);

  /* Create broker */
  broker = helicsCreateBroker("zmq","",initstring);

  isconnected = helicsBrokerisConnected(broker);

  if(isconnected) {
    printf("PI SENDER: Broker created and connected\n");
  }
  
  /* Create Federate Info object that describes the federate properties */
  fedinfo = createFederateInfoObject();
  
  /* Set Federate name */
  status = FederateInfoSetFederateName(fedinfo,"TestA Federate");

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
  printf("PI SENDER: Value federate created\n");

  /* Register the publication */
  pub = helicsRegisterGlobalPublication(vfed,"testA","double","");
  printf("PI SENDER: Publication registered\n");

  /* Enter execution mode */
  status = helicsEnterExecutionMode(vfed);
  printf("PI SENDER: Entering execution mode\n");

  /* This federate will be publishing deltat*pi for numsteps steps */
  double this_time = 0.0;
  double value = 22.0/7.0,val;
  helics_time_t currenttime;
  int           numsteps=20,i;

  for(i=0; i < numsteps; i++) {
    val = i*deltat*value;

    currenttime = helicsRequestTime(vfed,helicsTimeFromDouble(this_time+deltat*i));

    printf("PI SENDER: Sending value %3.2fpi = %4.3f at time %3.2f to PI RECEIVER\n",deltat*i,val,this_time+deltat*i);
    status = helicsPublishDouble(pub,val);
  }

  status = helicsFinalize(vfed);
  printf("PI SENDER: Federate finalized\n");


  while(helicsBrokerisConnected(broker)) {
#ifdef _MSC_VER
	  Sleep(1);
#else
    usleep(1000); /* Sleep for 1 millisecond */
#endif
  }
  printf("PI SENDER: Broker disconnected\n");

  return(0);
}
