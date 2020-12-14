/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <stdint.h>
#include <stdlib.h>

typedef enum {
   HELICS_CORE_TYPE_DEFAULT = 0,

   HELICS_CORE_TYPE_ZMQ = 1,

   HELICS_CORE_TYPE_MPI = 2,

   HELICS_CORE_TYPE_TEST = 3,

   HELICS_CORE_TYPE_INTERPROCESS = 4,
   HELICS_CORE_TYPE_IPC = 5,
   HELICS_CORE_TYPE_TCP = 6,

   HELICS_CORE_TYPE_UDP = 7,

   HELICS_CORE_TYPE_ZMQ_SS = 10,

   HELICS_CORE_TYPE_NNG = 9,

   HELICS_CORE_TYPE_TCP_SS = 11,

   HELICS_CORE_TYPE_HTTP = 12,

   HELICS_CORE_TYPE_WEBSOCKET = 14,

   HELICS_CORE_TYPE_INPROC = 18,

   HELICS_CORE_TYPE_NULL = 66
} HelicsCoreTypes;

typedef enum {
   HELICS_DATA_TYPE_STRING = 0,
   HELICS_DATA_TYPE_DOUBLE = 1,
   HELICS_DATA_TYPE_INT = 2,
   HELICS_DATA_TYPE_COMPLEX = 3,
   HELICS_DATA_TYPE_VECTOR = 4,
   HELICS_DATA_TYPE_COMPLEX_VECTOR = 5,
   HELICS_DATA_TYPE_NAMED_POINT = 6,
   HELICS_DATA_TYPE_BOOLEAN = 7,
   HELICS_DATA_TYPE_TIME = 8,
   HELICS_DATA_TYPE_RAW = 25,
   HELICS_DATA_TYPE_MULTI = 33,
   HELICS_DATA_TYPE_ANY = 25262
} HelicsDataTypes;

#define HELICS_DATA_TYPE_CHAR HELICS_DATA_TYPE_STRING

typedef enum {
   HELICS_FLAG_OBSERVER = 0,
   HELICS_FLAG_UNINTERRUPTIBLE = 1,
   HELICS_FLAG_INTERRUPTIBLE = 2,
   HELICS_FLAG_SOURCE_ONLY = 4,
   HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE = 6,
   HELICS_FLAG_ONLY_UPDATE_ON_CHANGE = 8,
   HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE = 10,
   HELICS_FLAG_RESTRICTIVE_TIME_POLICY = 11,
   HELICS_FLAG_ROLLBACK = 12,
   HELICS_FLAG_FORWARD_COMPUTE = 14,
   HELICS_FLAG_REALTIME = 16,
   HELICS_FLAG_SINGLE_THREAD_FEDERATE = 27,
   HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS = 67,
   HELICS_FLAG_STRICT_CONFIG_CHECKING = 75
} HelicsFederateFlags;

typedef enum {
   HELICS_FLAG_DELAY_INIT_ENTRY = 45,
   HELICS_FLAG_ENABLE_INIT_ENTRY = 47
} HelicsCoreFlags;

typedef enum {
   HELICS_FLAG_SLOW_RESPONDING = 29,
   HELICS_FLAG_DEBUGGING = 31,
   HELICS_FLAG_TERMINATE_ON_ERROR = 72,
   HELICS_FLAG_FORCE_LOGGING_FLUSH = 88,
   HELICS_FLAG_DUMPLOG = 89
} HelicsFlags;

typedef enum {
              HELICS_LOG_LEVEL_NO_PRINT = -1,
              HELICS_LOG_LEVEL_ERROR = 0,
              HELICS_LOG_LEVEL_WARNING = 1,
              HELICS_LOG_LEVEL_SUMMARY = 2,
              HELICS_LOG_LEVEL_CONNECTIONS = 3,
              HELICS_LOG_LEVEL_INTERFACES = 4,
              HELICS_LOG_LEVEL_TIMING = 5,
              HELICS_LOG_LEVEL_DATA = 6,
              HELICS_LOG_LEVEL_TRACE = 7
} HelicsLogLevels;

typedef enum {
   HELICS_ERROR_FATAL = -404,

   HELICS_ERROR_EXTERNAL_TYPE = -203,

   HELICS_ERROR_OTHER = -101,

   HELICS_ERROR_INSUFFICIENT_SPACE = -18,
   HELICS_ERROR_EXECUTION_FAILURE = -14,

   HELICS_ERROR_INVALID_FUNCTION_CALL = -10,

   HELICS_ERROR_INVALID_STATE_TRANSITION = -9,

   HELICS_WARNING = -8,

   HELICS_ERROR_SYSTEM_FAILURE = -6,

   HELICS_ERROR_DISCARD = -5,

   HELICS_ERROR_INVALID_ARGUMENT = -4,

   HELICS_ERROR_INVALID_OBJECT = -3,

   HELICS_ERROR_CONNECTION_FAILURE = -2,

   HELICS_ERROR_REGISTRATION_FAILURE = -1,

   HELICS_OK = 0
} HelicsErrorTypes;

typedef enum {
   HELICS_PROPERTY_TIME_DELTA = 137,
   HELICS_PROPERTY_TIME_PERIOD = 140,
   HELICS_PROPERTY_TIME_OFFSET = 141,
   HELICS_PROPERTY_TIME_RT_LAG = 143,
   HELICS_PROPERTY_TIME_RT_LEAD = 144,
   HELICS_PROPERTY_TIME_RT_TOLERANCE = 145,
   HELICS_PROPERTY_TIME_INPUT_DELAY = 148,
   HELICS_PROPERTY_TIME_OUTPUT_DELAY = 150,
   HELICS_PROPERTY_INT_MAX_ITERATIONS = 259,
   HELICS_PROPERTY_INT_LOG_LEVEL = 271,
   HELICS_PROPERTY_INT_FILE_LOG_LEVEL = 272,
   HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL = 274
} HelicsProperties;

typedef enum {
   HELICS_MULTI_INPUT_NO_OP = 0,
   HELICS_MULTI_INPUT_VECTORIZE_OPERATION = 1,
   HELICS_MULTI_INPUT_AND_OPERATION = 2,
   HELICS_MULTI_INPUT_OR_OPERATION = 3,
   HELICS_MULTI_INPUT_SUM_OPERATION = 4,
   HELICS_MULTI_INPUT_DIFF_OPERATION = 5,
   HELICS_MULTI_INPUT_MAX_OPERATION = 6,
   HELICS_MULTI_INPUT_MIN_OPERATION = 7,
   HELICS_MULTI_INPUT_AVERAGE_OPERATION = 8
} HelicsMultiInputModes;

typedef enum {
   HELICS_HANDLE_OPTION_CONNECTION_REQUIRED = 397,
   HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL = 402,
   HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY = 407,
   HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED = 409,
   HELICS_HANDLE_OPTION_BUFFER_DATA = 411,
   HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING = 414,
   HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH = 447,
   HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE = 452,
   HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE = 454,
   HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS = 475,
   HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD = 507,
   HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION = 510,
   HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST = 512,
   HELICS_HANDLE_OPTION_CONNECTIONS = 522
} HelicsHandleOptions;

typedef enum {
   HELICS_FILTER_TYPE_CUSTOM = 0,
   HELICS_FILTER_TYPE_DELAY = 1,
   HELICS_FILTER_TYPE_RANDOM_DELAY = 2,
   HELICS_FILTER_TYPE_RANDOM_DROP = 3,
   HELICS_FILTER_TYPE_REROUTE = 4,
   HELICS_FILTER_TYPE_CLONE = 5,
   HELICS_FILTER_TYPE_FIREWALL = 6
} HelicsFilterTypes;

typedef void* HelicsInput;
typedef void* HelicsInput;

typedef void* HelicsPublication;
typedef void* HelicsPublication;

typedef void* helics_endpoint;
typedef void* HelicsEndpoint;

typedef void* HelicsFilter;
typedef void* HelicsFilter;

typedef void* HelicsCore;
typedef void* HelicsCore;

typedef void* HelicsBroker;
typedef void* HelicsBroker;

typedef void* HelicsFederate;
typedef void* HelicsFederate;

typedef void* helics_federate_info;
typedef void* HelicsFederateInfo;

typedef void* HelicsQuery;
typedef void* HelicsQuery;

typedef void* helics_query_buffer;
typedef void* HelicsQueryBuffer;

typedef void* helics_message;
typedef void* HelicsMessage;

typedef double HelicsTime;
typedef double HelicsTime;

const HelicsTime HELICS_TIME_ZERO = 0.0;
const HelicsTime HELICS_TIME_EPSILON = 1.0e-9;
const HelicsTime HELICS_TIME_INVALID = -1.785e39;
const HelicsTime HELICS_TIME_MAXTIME = 9223372036.854774;

typedef int HelicsBool;
typedef int HelicsBool;

const HelicsBool HELICS_TRUE = 1;
const HelicsBool HELICS_FALSE = 0;

typedef enum {
   HELICS_ITERATION_REQUEST_NO_ITERATION,
   HELICS_ITERATION_REQUEST_FORCE_ITERATION,
   HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED
} HelicsIterationRequest;

typedef enum {
   HELICS_ITERATION_RESULT_NEXT_STEP,
   HELICS_ITERATION_RESULT_ERROR,
   HELICS_ITERATION_RESULT_HALTED,
   HELICS_ITERATION_RESULT_ITERATING
} HelicsIterationResult;

typedef enum {
   HELICS_STATE_STARTUP = 0,
   HELICS_STATE_INITIALIZATION,
   HELICS_STATE_EXECUTION,
   HELICS_STATE_FINALIZE,
   HELICS_STATE_ERROR,

   HELICS_STATE_PENDING_INIT,
   HELICS_STATE_PENDING_EXEC,
   HELICS_STATE_PENDING_TIME,
   HELICS_STATE_PENDING_ITERATIVE_TIME,
   HELICS_STATE_PENDING_FINALIZE
} HelicsFederateState;

typedef struct HelicsComplex {
   double real;
   double imag;
} HelicsComplex;

typedef HelicsComplex helics_complex;

typedef struct HelicsError {
   int32_t errorCode;
   const char* message;
} HelicsError;

typedef HelicsError HelicsError;

const char* helicsGetVersion(void);
const char* helicsGetBuildFlags(void);
const char* helicsGetCompilerVersion(void);
HelicsError helicsErrorInitialize(void);
void helicsErrorClear(HelicsError* err);
HelicsBool helicsIsCoreTypeAvailable(const char* type);
HelicsCore helicsCreateCore(const char* type, const char* name, const char* initString, HelicsError* err);
HelicsCore
    helicsCreateCoreFromArgs(const char* type, const char* name, int argc, const char* const* argv, HelicsError* err);
HelicsCore helicsCoreClone(HelicsCore core, HelicsError* err);
HelicsBool helicsCoreIsValid(HelicsCore core);
HelicsBroker helicsCreateBroker(const char* type, const char* name, const char* initString, HelicsError* err);
HelicsBroker
    helicsCreateBrokerFromArgs(const char* type, const char* name, int argc, const char* const* argv, HelicsError* err);
HelicsBroker helicsBrokerClone(HelicsBroker broker, HelicsError* err);
HelicsBool helicsBrokerIsValid(HelicsBroker broker);
HelicsBool helicsBrokerIsConnected(HelicsBroker broker);
void helicsBrokerDataLink(HelicsBroker broker, const char* source, const char* target, HelicsError* err);
void helicsBrokerAddSourceFilterToEndpoint(HelicsBroker broker, const char* filter, const char* endpoint, HelicsError* err);
void
    helicsBrokerAddDestinationFilterToEndpoint(HelicsBroker broker, const char* filter, const char* endpoint, HelicsError* err);
void helicsBrokerMakeConnections(HelicsBroker broker, const char* file, HelicsError* err);
HelicsBool helicsCoreWaitForDisconnect(HelicsCore core, int msToWait, HelicsError* err);
HelicsBool helicsBrokerWaitForDisconnect(HelicsBroker broker, int msToWait, HelicsError* err);
HelicsBool helicsCoreIsConnected(HelicsCore core);
void helicsCoreDataLink(HelicsCore core, const char* source, const char* target, HelicsError* err);
void helicsCoreAddSourceFilterToEndpoint(HelicsCore core, const char* filter, const char* endpoint, HelicsError* err);
void helicsCoreAddDestinationFilterToEndpoint(HelicsCore core, const char* filter, const char* endpoint, HelicsError* err);
void helicsCoreMakeConnections(HelicsCore core, const char* file, HelicsError* err);
const char* helicsBrokerGetIdentifier(HelicsBroker broker);
const char* helicsCoreGetIdentifier(HelicsCore core);
const char* helicsBrokerGetAddress(HelicsBroker broker);
const char* helicsCoreGetAddress(HelicsCore core);
void helicsCoreSetReadyToInit(HelicsCore core, HelicsError* err);
HelicsBool helicsCoreConnect(HelicsCore core, HelicsError* err);
void helicsCoreDisconnect(HelicsCore core, HelicsError* err);
HelicsFederate helicsGetFederateByName(const char* fedName, HelicsError* err);
void helicsBrokerDisconnect(HelicsBroker broker, HelicsError* err);
void helicsFederateDestroy(HelicsFederate fed);
void helicsBrokerDestroy(HelicsBroker broker);
void helicsCoreDestroy(HelicsCore core);
void helicsCoreFree(HelicsCore core);
void helicsBrokerFree(HelicsBroker broker);

HelicsFederate helicsCreateValueFederate(const char* fedName, helics_federate_info fi, HelicsError* err);
HelicsFederate helicsCreateValueFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsCreateMessageFederate(const char* fedName, helics_federate_info fi, HelicsError* err);
HelicsFederate helicsCreateMessageFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsCreateCombinationFederate(const char* fedName, helics_federate_info fi, HelicsError* err);
HelicsFederate helicsCreateCombinationFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsFederateClone(HelicsFederate fed, HelicsError* err);
helics_federate_info helicsCreateFederateInfo(void);
helics_federate_info helicsFederateInfoClone(helics_federate_info fi, HelicsError* err);
void helicsFederateInfoLoadFromArgs(helics_federate_info fi, int argc, const char* const* argv, HelicsError* err);
void helicsFederateInfoFree(helics_federate_info fi);
HelicsBool helicsFederateIsValid(HelicsFederate fed);
void helicsFederateInfoSetCoreName(helics_federate_info fi, const char* corename, HelicsError* err);
void helicsFederateInfoSetCoreInitString(helics_federate_info fi, const char* coreInit, HelicsError* err);
void helicsFederateInfoSetBrokerInitString(helics_federate_info fi, const char* brokerInit, HelicsError* err);
void helicsFederateInfoSetCoreType(helics_federate_info fi, int coretype, HelicsError* err);
void helicsFederateInfoSetCoreTypeFromString(helics_federate_info fi, const char* coretype, HelicsError* err);
void helicsFederateInfoSetBroker(helics_federate_info fi, const char* broker, HelicsError* err);
void helicsFederateInfoSetBrokerKey(helics_federate_info fi, const char* brokerkey, HelicsError* err);
void helicsFederateInfoSetBrokerPort(helics_federate_info fi, int brokerPort, HelicsError* err);
void helicsFederateInfoSetLocalPort(helics_federate_info fi, const char* localPort, HelicsError* err);
int helicsGetPropertyIndex(const char* val);
int helicsGetFlagIndex(const char* val);
int helicsGetOptionIndex(const char* val);
int helicsGetOptionValue(const char* val);
void helicsFederateInfoSetFlagOption(helics_federate_info fi, int flag, HelicsBool value, HelicsError* err);
void helicsFederateInfoSetSeparator(helics_federate_info fi, char separator, HelicsError* err);
void
    helicsFederateInfoSetTimeProperty(helics_federate_info fi, int timeProperty, HelicsTime propertyValue, HelicsError* err);
void helicsFederateInfoSetIntegerProperty(helics_federate_info fi, int intProperty, int propertyValue, HelicsError* err);
void helicsFederateRegisterInterfaces(HelicsFederate fed, const char* file, HelicsError* err);
void helicsFederateGlobalError(HelicsFederate fed, int error_code, const char* error_string);
void helicsFederateLocalError(HelicsFederate fed, int error_code, const char* error_string);
void helicsFederateFinalize(HelicsFederate fed, HelicsError* err);
void helicsFederateFinalizeAsync(HelicsFederate fed, HelicsError* err);
void helicsFederateFinalizeComplete(HelicsFederate fed, HelicsError* err);
void helicsFederateFree(HelicsFederate fed);
void helicsCloseLibrary(void);

void helicsFederateEnterInitializingMode(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterInitializingModeAsync(HelicsFederate fed, HelicsError* err);
HelicsBool helicsFederateIsAsyncOperationCompleted(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterInitializingModeComplete(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterExecutingMode(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterExecutingModeAsync(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterExecutingModeComplete(HelicsFederate fed, HelicsError* err);
HelicsIterationResult helicsFederateEnterExecutingModeIterative(HelicsFederate fed,
                                                                               HelicsIterationRequest iterate,
                                                                               HelicsError* err);
void helicsFederateEnterExecutingModeIterativeAsync(HelicsFederate fed, HelicsIterationRequest iterate, HelicsError* err);
HelicsIterationResult helicsFederateEnterExecutingModeIterativeComplete(HelicsFederate fed, HelicsError* err);
HelicsFederateState helicsFederateGetState(HelicsFederate fed, HelicsError* err);
HelicsCore helicsFederateGetCore(HelicsFederate fed, HelicsError* err);
HelicsTime helicsFederateRequestTime(HelicsFederate fed, HelicsTime requestTime, HelicsError* err);
HelicsTime helicsFederateRequestTimeAdvance(HelicsFederate fed, HelicsTime timeDelta, HelicsError* err);
HelicsTime helicsFederateRequestNextStep(HelicsFederate fed, HelicsError* err);
HelicsTime helicsFederateRequestTimeIterative(HelicsFederate fed,
                                                            HelicsTime requestTime,
                                                            HelicsIterationRequest iterate,
                                                            HelicsIterationResult* outIteration,
                                                            HelicsError* err);
void helicsFederateRequestTimeAsync(HelicsFederate fed, HelicsTime requestTime, HelicsError* err);
HelicsTime helicsFederateRequestTimeComplete(HelicsFederate fed, HelicsError* err);
void helicsFederateRequestTimeIterativeAsync(HelicsFederate fed,
                                                          HelicsTime requestTime,
                                                          HelicsIterationRequest iterate,
                                                          HelicsError* err);
HelicsTime helicsFederateRequestTimeIterativeComplete(HelicsFederate fed,
                                                                    HelicsIterationResult* outIterate,
                                                                    HelicsError* err);
const char* helicsFederateGetName(HelicsFederate fed);
void helicsFederateSetTimeProperty(HelicsFederate fed, int timeProperty, HelicsTime time, HelicsError* err);
void helicsFederateSetFlagOption(HelicsFederate fed, int flag, HelicsBool flagValue, HelicsError* err);
void helicsFederateSetSeparator(HelicsFederate fed, char separator, HelicsError* err);
void helicsFederateSetIntegerProperty(HelicsFederate fed, int intProperty, int propertyVal, HelicsError* err);
HelicsTime helicsFederateGetTimeProperty(HelicsFederate fed, int timeProperty, HelicsError* err);
HelicsBool helicsFederateGetFlagOption(HelicsFederate fed, int flag, HelicsError* err);
int helicsFederateGetIntegerProperty(HelicsFederate fed, int intProperty, HelicsError* err);
HelicsTime helicsFederateGetCurrentTime(HelicsFederate fed, HelicsError* err);
void helicsFederateSetGlobal(HelicsFederate fed, const char* valueName, const char* value, HelicsError* err);
void helicsFederateAddDependency(HelicsFederate fed, const char* fedName, HelicsError* err);
void helicsFederateSetLogFile(HelicsFederate fed, const char* logFile, HelicsError* err);
void helicsFederateLogErrorMessage(HelicsFederate fed, const char* logmessage, HelicsError* err);
void helicsFederateLogWarningMessage(HelicsFederate fed, const char* logmessage, HelicsError* err);
void helicsFederateLogInfoMessage(HelicsFederate fed, const char* logmessage, HelicsError* err);
void helicsFederateLogDebugMessage(HelicsFederate fed, const char* logmessage, HelicsError* err);
void helicsFederateLogLevelMessage(HelicsFederate fed, int loglevel, const char* logmessage, HelicsError* err);
void helicsFederateSendCommand(HelicsFederate fed, const char* target, const char* command, HelicsError* err);
const char* helicsFederateGetCommand(HelicsFederate fed, HelicsError* err);
const char* helicsFederateGetCommandSource(HelicsFederate fed, HelicsError* err);
const char* helicsFederateWaitCommand(HelicsFederate fed, HelicsError* err);
void helicsCoreSetGlobal(HelicsCore core, const char* valueName, const char* value, HelicsError* err);
void helicsBrokerSetGlobal(HelicsBroker broker, const char* valueName, const char* value, HelicsError* err);
void helicsCoreSendCommand(HelicsCore core, const char* target, const char* command, HelicsError* err);
void helicsBrokerSendCommand(HelicsBroker broker, const char* target, const char* command, HelicsError* err);
void helicsCoreSetLogFile(HelicsCore core, const char* logFileName, HelicsError* err);
void helicsBrokerSetLogFile(HelicsBroker broker, const char* logFileName, HelicsError* err);
void helicsBrokerSetTimeBarrier(HelicsBroker broker, HelicsTime barrierTime, HelicsError* err);
void helicsBrokerClearTimeBarrier(HelicsBroker broker);
HelicsQuery helicsCreateQuery(const char* target, const char* query);
const char* helicsQueryExecute(HelicsQuery query, HelicsFederate fed, HelicsError* err);
const char* helicsQueryCoreExecute(HelicsQuery query, HelicsCore core, HelicsError* err);
const char* helicsQueryBrokerExecute(HelicsQuery query, HelicsBroker broker, HelicsError* err);
void helicsQueryExecuteAsync(HelicsQuery query, HelicsFederate fed, HelicsError* err);
const char* helicsQueryExecuteComplete(HelicsQuery query, HelicsError* err);
HelicsBool helicsQueryIsCompleted(HelicsQuery query);
void helicsQuerySetTarget(HelicsQuery query, const char* target, HelicsError* err);
void helicsQuerySetQueryString(HelicsQuery query, const char* queryString, HelicsError* err);
void helicsQueryFree(HelicsQuery query);
void helicsCleanupLibrary(void);

HelicsInput helicsFederateRegisterSubscription(HelicsFederate fed, const char* key, const char* units, HelicsError* err);
HelicsPublication
    helicsFederateRegisterPublication(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);
HelicsPublication
    helicsFederateRegisterTypePublication(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);
HelicsPublication helicsFederateRegisterGlobalPublication(HelicsFederate fed,
                                                                        const char* key,
                                                                        HelicsDataTypes type,
                                                                        const char* units,
                                                                        HelicsError* err);
HelicsPublication helicsFederateRegisterGlobalTypePublication(HelicsFederate fed,
                                                                            const char* key,
                                                                            const char* type,
                                                                            const char* units,
                                                                            HelicsError* err);
HelicsInput
    helicsFederateRegisterInput(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);
HelicsInput
    helicsFederateRegisterTypeInput(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);
HelicsPublication
    helicsFederateRegisterGlobalInput(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);
HelicsPublication
    helicsFederateRegisterGlobalTypeInput(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);
HelicsPublication helicsFederateGetPublication(HelicsFederate fed, const char* key, HelicsError* err);
HelicsPublication helicsFederateGetPublicationByIndex(HelicsFederate fed, int index, HelicsError* err);
HelicsInput helicsFederateGetInput(HelicsFederate fed, const char* key, HelicsError* err);
HelicsInput helicsFederateGetInputByIndex(HelicsFederate fed, int index, HelicsError* err);
HelicsInput helicsFederateGetSubscription(HelicsFederate fed, const char* key, HelicsError* err);
void helicsFederateClearUpdates(HelicsFederate fed);
void helicsFederateRegisterFromPublicationJSON(HelicsFederate fed, const char* json, HelicsError* err);
void helicsFederatePublishJSON(HelicsFederate fed, const char* json, HelicsError* err);

HelicsBool helicsPublicationIsValid(HelicsPublication pub);
void helicsPublicationPublishBytes(HelicsPublication pub, const void* data, int inputDataLength, HelicsError* err);
void helicsPublicationPublishString(HelicsPublication pub, const char* str, HelicsError* err);
void helicsPublicationPublishInteger(HelicsPublication pub, int64_t val, HelicsError* err);
void helicsPublicationPublishBoolean(HelicsPublication pub, HelicsBool val, HelicsError* err);
void helicsPublicationPublishDouble(HelicsPublication pub, double val, HelicsError* err);
void helicsPublicationPublishTime(HelicsPublication pub, HelicsTime val, HelicsError* err);
void helicsPublicationPublishChar(HelicsPublication pub, char val, HelicsError* err);
void helicsPublicationPublishComplex(HelicsPublication pub, double real, double imag, HelicsError* err);
void helicsPublicationPublishVector(HelicsPublication pub, const double* vectorInput, int vectorLength, HelicsError* err);
void helicsPublicationPublishNamedPoint(HelicsPublication pub, const char* str, double val, HelicsError* err);
void helicsPublicationAddTarget(HelicsPublication pub, const char* target, HelicsError* err);
HelicsBool helicsInputIsValid(HelicsInput ipt);
void helicsInputAddTarget(HelicsInput ipt, const char* target, HelicsError* err);

int helicsInputGetByteCount(HelicsInput ipt);
void helicsInputGetBytes(HelicsInput ipt, void* data, int maxDataLength, int* actualSize, HelicsError* err);
int helicsInputGetStringSize(HelicsInput ipt);
void helicsInputGetString(HelicsInput ipt, char* outputString, int maxStringLength, int* actualLength, HelicsError* err);
int64_t helicsInputGetInteger(HelicsInput ipt, HelicsError* err);
HelicsBool helicsInputGetBoolean(HelicsInput ipt, HelicsError* err);
double helicsInputGetDouble(HelicsInput ipt, HelicsError* err);
HelicsTime helicsInputGetTime(HelicsInput ipt, HelicsError* err);
char helicsInputGetChar(HelicsInput ipt, HelicsError* err);
helics_complex helicsInputGetComplexObject(HelicsInput ipt, HelicsError* err);
void helicsInputGetComplex(HelicsInput ipt, double* real, double* imag, HelicsError* err);
int helicsInputGetVectorSize(HelicsInput ipt);
void helicsInputGetVector(HelicsInput ipt, double data[], int maxLength, int* actualSize, HelicsError* err);
void
    helicsInputGetNamedPoint(HelicsInput ipt, char* outputString, int maxStringLength, int* actualLength, double* val, HelicsError* err);

void helicsInputSetDefaultBytes(HelicsInput ipt, const void* data, int inputDataLength, HelicsError* err);
void helicsInputSetDefaultString(HelicsInput ipt, const char* str, HelicsError* err);
void helicsInputSetDefaultInteger(HelicsInput ipt, int64_t val, HelicsError* err);
void helicsInputSetDefaultBoolean(HelicsInput ipt, HelicsBool val, HelicsError* err);
void helicsInputSetDefaultTime(HelicsInput ipt, HelicsTime val, HelicsError* err);
void helicsInputSetDefaultChar(HelicsInput ipt, char val, HelicsError* err);
void helicsInputSetDefaultDouble(HelicsInput ipt, double val, HelicsError* err);
void helicsInputSetDefaultComplex(HelicsInput ipt, double real, double imag, HelicsError* err);
void helicsInputSetDefaultVector(HelicsInput ipt, const double* vectorInput, int vectorLength, HelicsError* err);
void helicsInputSetDefaultNamedPoint(HelicsInput ipt, const char* str, double val, HelicsError* err);

const char* helicsInputGetType(HelicsInput ipt);
const char* helicsInputGetPublicationType(HelicsInput ipt);
const char* helicsPublicationGetType(HelicsPublication pub);
const char* helicsInputGetKey(HelicsInput ipt);
const char* helicsSubscriptionGetKey(HelicsInput ipt);
const char* helicsPublicationGetKey(HelicsPublication pub);
const char* helicsInputGetUnits(HelicsInput ipt);
const char* helicsInputGetInjectionUnits(HelicsInput ipt);
const char* helicsInputGetExtractionUnits(HelicsInput ipt);
const char* helicsPublicationGetUnits(HelicsPublication pub);
const char* helicsInputGetInfo(HelicsInput inp);
void helicsInputSetInfo(HelicsInput inp, const char* info, HelicsError* err);
const char* helicsPublicationGetInfo(HelicsPublication pub);
void helicsPublicationSetInfo(HelicsPublication pub, const char* info, HelicsError* err);
int helicsInputGetOption(HelicsInput inp, int option);
void helicsInputSetOption(HelicsInput inp, int option, int value, HelicsError* err);
int helicsPublicationGetOption(HelicsPublication pub, int option);
void helicsPublicationSetOption(HelicsPublication pub, int option, int val, HelicsError* err);
void helicsPublicationSetMinimumChange(HelicsPublication pub, double tolerance, HelicsError* err);
void helicsInputSetMinimumChange(HelicsInput inp, double tolerance, HelicsError* err);

HelicsBool helicsInputIsUpdated(HelicsInput ipt);
HelicsTime helicsInputLastUpdateTime(HelicsInput ipt);
void helicsInputClearUpdate(HelicsInput ipt);
int helicsFederateGetPublicationCount(HelicsFederate fed);
int helicsFederateGetInputCount(HelicsFederate fed);

helics_endpoint helicsFederateRegisterEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err);
helics_endpoint helicsFederateRegisterGlobalEndpoint(HelicsFederate fed,
                                                                  const char* name,
                                                                  const char* type,
                                                                  HelicsError* err);
helics_endpoint helicsFederateRegisterTargetedEndpoint(HelicsFederate fed,
                                                                    const char* name,
                                                                    const char* type,
                                                                    HelicsError* err);
helics_endpoint helicsFederateRegisterGlobalTargetedEndpoint(HelicsFederate fed,
                                                                          const char* name,
                                                                          const char* type,
                                                                          HelicsError* err);
helics_endpoint helicsFederateGetEndpoint(HelicsFederate fed, const char* name, HelicsError* err);
helics_endpoint helicsFederateGetEndpointByIndex(HelicsFederate fed, int index, HelicsError* err);
HelicsBool helicsEndpointIsValid(helics_endpoint endpoint);
void helicsEndpointSetDefaultDestination(helics_endpoint endpoint, const char* dst, HelicsError* err);
const char* helicsEndpointGetDefaultDestination(helics_endpoint endpoint);
void helicsEndpointSendBytes(helics_endpoint endpoint, const void* data, int inputDataLength, HelicsError* err);
void
    helicsEndpointSendBytesTo(helics_endpoint endpoint, const void* data, int inputDataLength, const char* dst, HelicsError* err);
void helicsEndpointSendBytesToAt(helics_endpoint endpoint,
                                              const void* data,
                                              int inputDataLength,
                                              const char* dst,
                                              HelicsTime time,
                                              HelicsError* err);
void
    helicsEndpointSendBytesAt(helics_endpoint endpoint, const void* data, int inputDataLength, HelicsTime time, HelicsError* err);
void helicsEndpointSendMessage(helics_endpoint endpoint, helics_message message, HelicsError* err);
void helicsEndpointSendMessageZeroCopy(helics_endpoint endpoint, helics_message message, HelicsError* err);
void helicsEndpointSubscribe(helics_endpoint endpoint, const char* key, HelicsError* err);
HelicsBool helicsFederateHasMessage(HelicsFederate fed);
HelicsBool helicsEndpointHasMessage(helics_endpoint endpoint);
int helicsFederatePendingMessagesCount(HelicsFederate fed);
int helicsEndpointPendingMessagesCount(helics_endpoint endpoint);
helics_message helicsEndpointGetMessage(helics_endpoint endpoint);
helics_message helicsEndpointCreateMessage(helics_endpoint endpoint, HelicsError* err);
helics_message helicsFederateGetMessage(HelicsFederate fed);
helics_message helicsFederateCreateMessage(HelicsFederate fed, HelicsError* err);
void helicsFederateClearMessages(HelicsFederate fed);
const char* helicsEndpointGetType(helics_endpoint endpoint);
const char* helicsEndpointGetName(helics_endpoint endpoint);
int helicsFederateGetEndpointCount(HelicsFederate fed);
const char* helicsEndpointGetInfo(helics_endpoint end);
void helicsEndpointSetInfo(helics_endpoint endpoint, const char* info, HelicsError* err);
void helicsEndpointSetOption(helics_endpoint endpoint, int option, int value, HelicsError* err);
int helicsEndpointGetOption(helics_endpoint endpoint, int option);
void helicsEndpointAddSourceTarget(helics_endpoint endpoint, const char* targetEndpoint, HelicsError* err);
void helicsEndpointAddDestinationTarget(helics_endpoint endpoint, const char* targetEndpoint, HelicsError* err);
void helicsEndpointRemoveTarget(helics_endpoint endpoint, const char* targetEndpoint, HelicsError* err);
void helicsEndpointAddSourceFilter(helics_endpoint endpoint, const char* filterName, HelicsError* err);
void helicsEndpointAddDestinationFilter(helics_endpoint endpoint, const char* filterName, HelicsError* err);

const char* helicsMessageGetSource(helics_message message);
const char* helicsMessageGetDestination(helics_message message);
const char* helicsMessageGetOriginalSource(helics_message message);
const char* helicsMessageGetOriginalDestination(helics_message message);
HelicsTime helicsMessageGetTime(helics_message message);
const char* helicsMessageGetString(helics_message message);
int helicsMessageGetMessageID(helics_message message);
HelicsBool helicsMessageGetFlagOption(helics_message message, int flag);
int helicsMessageGetByteCount(helics_message message);
void helicsMessageGetBytes(helics_message message, void* data, int maxMessageLength, int* actualSize, HelicsError* err);
void* helicsMessageGetBytesPointer(helics_message message);
HelicsBool helicsMessageIsValid(helics_message message);
void helicsMessageSetSource(helics_message message, const char* src, HelicsError* err);
void helicsMessageSetDestination(helics_message message, const char* dst, HelicsError* err);
void helicsMessageSetOriginalSource(helics_message message, const char* src, HelicsError* err);
void helicsMessageSetOriginalDestination(helics_message message, const char* dst, HelicsError* err);
void helicsMessageSetTime(helics_message message, HelicsTime time, HelicsError* err);
void helicsMessageResize(helics_message message, int newSize, HelicsError* err);
void helicsMessageReserve(helics_message message, int reserveSize, HelicsError* err);
void helicsMessageSetMessageID(helics_message message, int32_t messageID, HelicsError* err);
void helicsMessageClearFlags(helics_message message);
void helicsMessageSetFlagOption(helics_message message, int flag, HelicsBool flagValue, HelicsError* err);
void helicsMessageSetString(helics_message message, const char* str, HelicsError* err);
void helicsMessageSetData(helics_message message, const void* data, int inputDataLength, HelicsError* err);
void helicsMessageAppendData(helics_message message, const void* data, int inputDataLength, HelicsError* err);
void helicsMessageCopy(helics_message src_message, helics_message dst_message, HelicsError* err);
helics_message helicsMessageClone(helics_message message, HelicsError* err);
void helicsMessageFree(helics_message message);
void helicsMessageClear(helics_message message, HelicsError* err);

HelicsFilter helicsFederateRegisterFilter(HelicsFederate fed, HelicsFilterTypes type, const char* name, HelicsError* err);
HelicsFilter helicsFederateRegisterGlobalFilter(HelicsFederate fed,
                                                              HelicsFilterTypes type,
                                                              const char* name,
                                                              HelicsError* err);
HelicsFilter helicsFederateRegisterCloningFilter(HelicsFederate fed, const char* name, HelicsError* err);
HelicsFilter helicsFederateRegisterGlobalCloningFilter(HelicsFederate fed, const char* name, HelicsError* err);
HelicsFilter helicsCoreRegisterFilter(HelicsCore core, HelicsFilterTypes type, const char* name, HelicsError* err);
HelicsFilter helicsCoreRegisterCloningFilter(HelicsCore core, const char* name, HelicsError* err);
int helicsFederateGetFilterCount(HelicsFederate fed);
HelicsFilter helicsFederateGetFilter(HelicsFederate fed, const char* name, HelicsError* err);
HelicsFilter helicsFederateGetFilterByIndex(HelicsFederate fed, int index, HelicsError* err);
HelicsBool helicsFilterIsValid(HelicsFilter filt);
const char* helicsFilterGetName(HelicsFilter filt);
void helicsFilterSet(HelicsFilter filt, const char* prop, double val, HelicsError* err);
void helicsFilterSetString(HelicsFilter filt, const char* prop, const char* val, HelicsError* err);
void helicsFilterAddDestinationTarget(HelicsFilter filt, const char* dst, HelicsError* err);
void helicsFilterAddSourceTarget(HelicsFilter filt, const char* source, HelicsError* err);

void helicsFilterAddDeliveryEndpoint(HelicsFilter filt, const char* deliveryEndpoint, HelicsError* err);
void helicsFilterRemoveTarget(HelicsFilter filt, const char* target, HelicsError* err);
void helicsFilterRemoveDeliveryEndpoint(HelicsFilter filt, const char* deliveryEndpoint, HelicsError* err);
const char* helicsFilterGetInfo(HelicsFilter filt);
void helicsFilterSetInfo(HelicsFilter filt, const char* info, HelicsError* err);
void helicsFilterSetOption(HelicsFilter filt, int option, int value, HelicsError* err);
int helicsFilterGetOption(HelicsFilter filt, int option);

void helicsBrokerSetLoggingCallback(HelicsBroker broker,
                                                 void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                                 void* userdata,
                                                 HelicsError* err);
void helicsCoreSetLoggingCallback(HelicsCore core,
                                               void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                               void* userdata,
                                               HelicsError* err);
void
    helicsFederateSetLoggingCallback(HelicsFederate fed,
                                    void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                    void* userdata,
                                    HelicsError* err);
void helicsFilterSetCustomCallback(HelicsFilter filter,
                                                void (*filtCall)(helics_message message, void* userData),
                                                void* userdata,
                                                HelicsError* err);
void
    helicsFederateSetQueryCallback(HelicsFederate fed,
                                  void (*queryAnswer)(const char* query, int querySize, helics_query_buffer buffer, void* userdata),
                                  void* userdata,
                                  HelicsError* err);
void helicsQueryBufferFill(helics_query_buffer buffer, const char* str, int strSize, HelicsError* err);

