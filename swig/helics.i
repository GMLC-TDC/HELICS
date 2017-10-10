#define __attribute__(x)

#define HELICS_Export __attribute__ ((visibility ("default")))

%module helics
%{
#include "helics.h"
%}

HELICS_Export const char *helicsGetVersion ();

HELICS_Export helics_core helicsCreateCore (const char *type, const char *name, const char *initString);

HELICS_Export helics_core helicsCreateCoreFromArgs (const char *type, const char *name, int argc, char *argv[]);

HELICS_Export int helicsBrokerIsConnected(helics_broker broker);

HELICS_Export int helicsCoreIsConnected(helics_core core);

HELICS_Export void helicsFreeCore(helics_core core);

HELICS_Export void helicsFreeBroker(helics_broker broker);

/* Creation and destruction of Federates */
HELICS_Export helics_value_federate helicsCreateValueFederate (const helics_federate_info_t fi);
HELICS_Export helics_value_federate helicsCreateValueFederateFromFile (const char *);

HELICS_Export helics_federate helicsCreateCombinationFederate (const helics_federate_info_t fi);
HELICS_Export helics_federate helicsCreateCombinationFederateFromFile (const char *filename);

HELICS_Export helics_federate_info_t helicsFederateInfoCreate ();
HELICS_Export void helicsFederateInfoFree (const helics_federate_info_t fi);

HELICS_Export helicsStatus helicsFederateInfoSetFederateName (helics_federate_info_t fi, const char *name);
HELICS_Export helicsStatus helicsFederateInfoSetCoreName (helics_federate_info_t fi, const char *corename);
HELICS_Export helicsStatus helicsFederateInfoSetCoreInitString (helics_federate_info_t fi, const char *coreInit);
HELICS_Export helicsStatus helicsFederateInfoSetCoreTypeFromString (helics_federate_info_t fi, const char *coretype);
HELICS_Export helicsStatus helicsFederateInfoSetCoreType (helics_federate_info_t fi, int coretype);
HELICS_Export helicsStatus helicsFederateInfoSetFlag (helics_federate_info_t fi, int flag, int value);
HELICS_Export helicsStatus helicsFederateInfoSetLookahead (helics_federate_info_t fi, helics_time_t lookahead);
HELICS_Export helicsStatus helicsFederateInfoSetTimeDelta (helics_federate_info_t fi, helics_time_t timeDelta);
HELICS_Export helicsStatus helicsFederateInfoSetImpactWindow (helics_federate_info_t fi, helics_time_t impactWindow);
HELICS_Export helicsStatus helicsFederateInfoSetTimeOffset (helics_federate_info_t fi, helics_time_t timeOffset);
HELICS_Export helicsStatus helicsFederateInfoSetPeriod (helics_federate_info_t fi, helics_time_t period);
HELICS_Export helicsStatus helicsFederateInfoSetMaxIterations (helics_federate_info_t fi, int max_iterations);
HELICS_Export helicsStatus helicsFederateInfoSetLoggingLevel (helics_federate_info_t fi, int logLevel);

HELICS_Export helicsStatus helicsFinalize (helics_federate fed);

/* initialization, execution, and time requests */
HELICS_Export helicsStatus helicsEnterInitializationMode (helics_federate fed);

HELICS_Export helicsStatus helicsEnterInitializationModeAsync(helics_federate fed);

HELICS_Export int helicsAsyncOperationCompleted(helics_federate fed);

HELICS_Export helicsStatus helicsEneterInitializationModeFinalize(helics_federate fed);

HELICS_Export helicsStatus helicsEnterExecutionMode (helics_federate fed);
HELICS_Export helicsStatus helicsEnterExecutionModeIterative (helics_federate fed,
                                                              convergence_status converged,
                                                              convergence_status *outConverged);

HELICS_Export helicsStatus helicsEnterExecutionModeAsync(helics_federate fed);
HELICS_Export helicsStatus helicsEnterExecutionModeIterativeAsync(helics_federate fed, convergence_status converged);

HELICS_Export helicsStatus helicsEnterExecutionModeFinalize(helics_federate fed);
HELICS_Export helicsStatus helicsEnterExecutionModeIterativeFinalize(helics_federate fed, convergence_status *outConverged);


HELICS_Export helics_time_t helicsRequestTime (helics_federate fed, helics_time_t requestTime);
HELICS_Export helics_iterative_time helicsRequestTimeIterative (helics_federate fed,
                                                                helics_time_t requestTime,
                                                                convergence_status converged);

HELICS_Export helicsStatus helicsRequestTimeAsync(helics_federate fed, helics_time_t requestTime);
HELICS_Export helicsStatus helicsRequestTimeIterativeAsync(helics_federate fed, helics_time_t requestTime, convergence_status converged);
HELICS_Export helics_time_t helicsRequestTimeFinalize(helics_federate fed);
HELICS_Export helics_iterative_time helicsRequestTimeIterativeFinalize(helics_federate fed);


HELICS_Export helics_query helicsCreateQuery(const char *target, const char *query);
HELICS_Export const char *helicsExecuteQuery(helics_federate fed, helics_query query);

HELICS_Export void helicsFreeQuery(helics_query);
HELICS_Export void helicsFreeFederate (helics_federate fed);


