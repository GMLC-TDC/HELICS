#include <stdio.h>
#include <math.h>
#include <cpp98/ValueFederate.hpp>
#include <cpp98/helics.hpp> // getHelicsVersionString

int main(int argc,char **argv)
{
  const char*    fedinitstring="--broker=mainbroker --federates=1";
  double         deltat=0.01;
  helics_subscription sub;
  helics_publication  pub;


  std::string helicsversion = helics::getHelicsVersionString();

  printf(" Helics version = %s\n",helicsversion.c_str());

  /* Create Federate Info object that describes the federate properties
   * Set federate name and core type from string
   */
  helics::FederateInfo fi ("TestB Federate", "zmq");

  /* Federate init string */
  fi.setCoreInitString (fedinitstring);

  fi.setTimeDelta (deltat);

  fi.setMaxIterations (100);

  fi.setLoggingLevel (1);

  /* Create value federate */
  helics::ValueFederate* vfed = new helics::ValueFederate (fi);
  printf(" Value federate created\n");

  sub = vfed->registerSubscription("testA","double");
  printf(" Subscription registered\n");

  /* Register the publication */
  pub = vfed->registerGlobalPublication("testB","double");
  printf(" Publication registered\n");


  /* Enter initialization state */
  vfed->enterInitializationState(); // can throw helics::InvalidStateTransition exception
  printf(" Entered initialization state\n");
  double y = 1.0, x = 0, xprv = 100,yprv=100;

  vfed->publish(pub, y);
  fflush(NULL);
  /* Enter execution state */
  vfed->enterExecutionState(); // can throw helics::InvalidStateTransition exception
  printf(" Entered execution state\n");

  fflush(NULL);
  helics_time_t currenttime=0.0;
  helics_iterative_time currenttimeiter;
  currenttimeiter.status = iterating;

  int           isupdated;
  double tol=1E-8;
  int helics_iter = 0;
  while (currenttimeiter.status==iterating)
  {

    xprv = x;
    x = vfed->getDouble(sub);
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
      vfed->publish(pub,y);
      printf("Fed2: publishing y\n");
    }
    fflush(NULL);
    currenttimeiter = vfed->requestTimeIterative(currenttime, iterate_if_needed);
    yprv = y;
  }

  vfed->finalize();
  printf("NLIN2: Federate finalized\n");
  fflush(NULL);
  // Destructor for ValueFederate must be called before close library
  delete vfed;
  helicsCloseLibrary();
  printf("NLIN2: Library Closed\n");
  fflush(NULL);
  return(0);

}
