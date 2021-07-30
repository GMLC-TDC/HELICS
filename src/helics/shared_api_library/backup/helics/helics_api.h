/*
Copyright (c) 2017-2021,
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
    HELICS_FLAG_STRICT_CONFIG_CHECKING = 75,
    HELICS_FLAG_EVENT_TRIGGERED = 81
} HelicsFederateFlags;

typedef enum { HELICS_FLAG_DELAY_INIT_ENTRY = 45, HELICS_FLAG_ENABLE_INIT_ENTRY = 47 } HelicsCoreFlags;

typedef enum {
    HELICS_FLAG_SLOW_RESPONDING = 29,
    HELICS_FLAG_DEBUGGING = 31,
    HELICS_FLAG_TERMINATE_ON_ERROR = 72,
    HELICS_FLAG_FORCE_LOGGING_FLUSH = 88,
    HELICS_FLAG_DUMPLOG = 89
} HelicsFlags;

typedef enum {
    HELICS_LOG_LEVEL_NO_PRINT = -4,
    HELICS_LOG_LEVEL_ERROR = 0,
    HELICS_LOG_LEVEL_WARNING = 3,
    HELICS_LOG_LEVEL_SUMMARY = 6,
    HELICS_LOG_LEVEL_CONNECTIONS = 9,
    HELICS_LOG_LEVEL_INTERFACES = 12,
    HELICS_LOG_LEVEL_TIMING = 15,
    HELICS_LOG_LEVEL_DATA = 18,
    HELICS_LOG_LEVEL_DEBUG = 21,
    HELICS_LOG_LEVEL_TRACE = 24
} HelicsLogLevels;

typedef enum {
    HELICS_ERROR_FATAL = -404,
    HELICS_ERROR_EXTERNAL_TYPE = -203,
    HELICS_ERROR_OTHER = -101,
    HELICS_ERROR_USER_ABORT = -27,
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

const int HELICS_INVALID_OPTION_INDEX = -101;

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

typedef enum { HELICS_SEQUENCING_MODE_FAST = 0, HELICS_SEQUENCING_MODE_ORDERED = 1 } HelicsSequencingModes;

typedef void* HelicsInput;

typedef void* HelicsPublication;

typedef void* HelicsEndpoint;

typedef void* HelicsFilter;

typedef void* HelicsCore;

typedef void* HelicsBroker;

typedef void* HelicsFederate;

typedef void* HelicsFederateInfo;

typedef void* HelicsQuery;

typedef void* HelicsQueryBuffer;

typedef void* HelicsMessage;

typedef double HelicsTime;

const HelicsTime HELICS_TIME_ZERO = 0.0;
const HelicsTime HELICS_TIME_EPSILON = 1.0e-9;
const HelicsTime HELICS_TIME_INVALID = -1.785e39;
const HelicsTime HELICS_TIME_MAXTIME = 9223372036.854774;

typedef int HelicsBool;

const HelicsBool HELICS_TRUE = 1;
const HelicsBool HELICS_FALSE = 0;

typedef enum {
    HELICS_ITERATION_REQUEST_NO_ITERATION = 0,
    HELICS_ITERATION_REQUEST_FORCE_ITERATION = 1,
    HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED = 2
} HelicsIterationRequest;

typedef enum {
    HELICS_ITERATION_RESULT_NEXT_STEP = 0,
    HELICS_ITERATION_RESULT_ERROR = 1,
    HELICS_ITERATION_RESULT_HALTED = 2,
    HELICS_ITERATION_RESULT_ITERATING = 3
} HelicsIterationResult;

typedef enum {
    HELICS_STATE_STARTUP = 0,
    HELICS_STATE_INITIALIZATION = 1,
    HELICS_STATE_EXECUTION = 2,
    HELICS_STATE_FINALIZE = 3,
    HELICS_STATE_ERROR = 4,

    HELICS_STATE_PENDING_INIT = 5,
    HELICS_STATE_PENDING_EXEC = 6,
    HELICS_STATE_PENDING_TIME = 7,
    HELICS_STATE_PENDING_ITERATIVE_TIME = 8,
    HELICS_STATE_PENDING_FINALIZE = 9,
    HELICS_STATE_FINISHED = 10
} HelicsFederateState;

typedef struct HelicsComplex {
    double real;
    double imag;
} HelicsComplex;

typedef struct HelicsError {
    int32_t error_code;
    const char* message;
} HelicsError;

const char* helicsGetVersion(void);
const char* helicsGetBuildFlags(void);
const char* helicsGetCompilerVersion(void);
HelicsError helicsErrorInitialize(void);
void helicsErrorClear(HelicsError* err);
void helicsLoadSignalHandler();
void helicsClearSignalHandler();
void helicsLoadSignalHandlerCallback(HelicsBool (*handler)(int));
void helicsAbort(int errorCode, const char* errorString);
HelicsBool helicsIsCoreTypeAvailable(const char* type);
HelicsCore helicsCreateCore(const char* type, const char* name, const char* initString, HelicsError* err);
HelicsCore helicsCreateCoreFromArgs(const char* type, const char* name, int argc, const char* const* argv, HelicsError* err);
HelicsCore helicsCoreClone(HelicsCore core, HelicsError* err);
HelicsBool helicsCoreIsValid(HelicsCore core);
HelicsBroker helicsCreateBroker(const char* type, const char* name, const char* initString, HelicsError* err);
HelicsBroker helicsCreateBrokerFromArgs(const char* type, const char* name, int argc, const char* const* argv, HelicsError* err);
HelicsBroker helicsBrokerClone(HelicsBroker broker, HelicsError* err);
HelicsBool helicsBrokerIsValid(HelicsBroker broker);
HelicsBool helicsBrokerIsConnected(HelicsBroker broker);
void helicsBrokerDataLink(HelicsBroker broker, const char* source, const char* target, HelicsError* err);
void helicsBrokerAddSourceFilterToEndpoint(HelicsBroker broker, const char* filter, const char* endpoint, HelicsError* err);
void helicsBrokerAddDestinationFilterToEndpoint(HelicsBroker broker, const char* filter, const char* endpoint, HelicsError* err);
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

HelicsFederate helicsCreateValueFederate(const char* fedName, HelicsFederateInfo fi, HelicsError* err);
HelicsFederate helicsCreateValueFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsCreateMessageFederate(const char* fedName, HelicsFederateInfo fi, HelicsError* err);
HelicsFederate helicsCreateMessageFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsCreateCombinationFederate(const char* fedName, HelicsFederateInfo fi, HelicsError* err);
HelicsFederate helicsCreateCombinationFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsFederateClone(HelicsFederate fed, HelicsError* err);
HelicsFederateInfo helicsCreateFederateInfo(void);
HelicsFederateInfo helicsFederateInfoClone(HelicsFederateInfo fi, HelicsError* err);
void helicsFederateInfoLoadFromArgs(HelicsFederateInfo fi, int argc, const char* const* argv, HelicsError* err);
void helicsFederateInfoFree(HelicsFederateInfo fi);
HelicsBool helicsFederateIsValid(HelicsFederate fed);
void helicsFederateInfoSetCoreName(HelicsFederateInfo fi, const char* corename, HelicsError* err);
void helicsFederateInfoSetCoreInitString(HelicsFederateInfo fi, const char* coreInit, HelicsError* err);
void helicsFederateInfoSetBrokerInitString(HelicsFederateInfo fi, const char* brokerInit, HelicsError* err);
void helicsFederateInfoSetCoreType(HelicsFederateInfo fi, int coretype, HelicsError* err);
void helicsFederateInfoSetCoreTypeFromString(HelicsFederateInfo fi, const char* coretype, HelicsError* err);
void helicsFederateInfoSetBroker(HelicsFederateInfo fi, const char* broker, HelicsError* err);
void helicsFederateInfoSetBrokerKey(HelicsFederateInfo fi, const char* brokerkey, HelicsError* err);
void helicsFederateInfoSetBrokerPort(HelicsFederateInfo fi, int brokerPort, HelicsError* err);
void helicsFederateInfoSetLocalPort(HelicsFederateInfo fi, const char* localPort, HelicsError* err);
int helicsGetPropertyIndex(const char* val);
int helicsGetFlagIndex(const char* val);
int helicsGetOptionIndex(const char* val);
int helicsGetOptionValue(const char* val);
void helicsFederateInfoSetFlagOption(HelicsFederateInfo fi, int flag, HelicsBool value, HelicsError* err);
void helicsFederateInfoSetSeparator(HelicsFederateInfo fi, char separator, HelicsError* err);
void helicsFederateInfoSetTimeProperty(HelicsFederateInfo fi, int timeProperty, HelicsTime propertyValue, HelicsError* err);
void helicsFederateInfoSetIntegerProperty(HelicsFederateInfo fi, int intProperty, int propertyValue, HelicsError* err);
void helicsFederateRegisterInterfaces(HelicsFederate fed, const char* file, HelicsError* err);
void helicsFederateGlobalError(HelicsFederate fed, int errorCode, const char* errorString, HelicsError* err);
void helicsFederateLocalError(HelicsFederate fed, int errorCode, const char* errorString, HelicsError* err);
void helicsFederateFinalize(HelicsFederate fed, HelicsError* err);
void helicsFederateFinalizeAsync(HelicsFederate fed, HelicsError* err);
void helicsFederateFinalizeComplete(HelicsFederate fed, HelicsError* err);
void helicsFederateDisconnect(HelicsFederate fed, HelicsError* err);
void helicsFederateDisconnectAsync(HelicsFederate fed, HelicsError* err);
void helicsFederateDisconnectComplete(HelicsFederate fed, HelicsError* err);
void helicsFederateFree(HelicsFederate fed);
void helicsCloseLibrary(void);

void helicsFederateEnterInitializingMode(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterInitializingModeAsync(HelicsFederate fed, HelicsError* err);
HelicsBool helicsFederateIsAsyncOperationCompleted(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterInitializingModeComplete(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterExecutingMode(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterExecutingModeAsync(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterExecutingModeComplete(HelicsFederate fed, HelicsError* err);
HelicsIterationResult helicsFederateEnterExecutingModeIterative(HelicsFederate fed, HelicsIterationRequest iterate, HelicsError* err);
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
void helicsFederateRequestTimeIterativeAsync(HelicsFederate fed, HelicsTime requestTime, HelicsIterationRequest iterate, HelicsError* err);
HelicsTime helicsFederateRequestTimeIterativeComplete(HelicsFederate fed, HelicsIterationResult* outIterate, HelicsError* err);
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
void helicsFederateSetTag(HelicsFederate fed, const char* tagName, const char* value, HelicsError* err);
const char* helicsFederateGetTag(HelicsFederate fed, const char* tagName, HelicsError* err);
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
void helicsBrokerGlobalError(HelicsBroker broker, int errorCode, const char* errorString, HelicsError* err);
void helicsCoreGlobalError(HelicsCore core, int errorCode, const char* errorString, HelicsError* err);
HelicsQuery helicsCreateQuery(const char* target, const char* query);
const char* helicsQueryExecute(HelicsQuery query, HelicsFederate fed, HelicsError* err);
const char* helicsQueryCoreExecute(HelicsQuery query, HelicsCore core, HelicsError* err);
const char* helicsQueryBrokerExecute(HelicsQuery query, HelicsBroker broker, HelicsError* err);
void helicsQueryExecuteAsync(HelicsQuery query, HelicsFederate fed, HelicsError* err);
const char* helicsQueryExecuteComplete(HelicsQuery query, HelicsError* err);
HelicsBool helicsQueryIsCompleted(HelicsQuery query);
void helicsQuerySetTarget(HelicsQuery query, const char* target, HelicsError* err);
void helicsQuerySetQueryString(HelicsQuery query, const char* queryString, HelicsError* err);
void helicsQuerySetOrdering(HelicsQuery query, int32_t mode, HelicsError* err);
void helicsQueryFree(HelicsQuery query);
void helicsCleanupLibrary(void);

HelicsInput helicsFederateRegisterSubscription(HelicsFederate fed, const char* key, const char* units, HelicsError* err);
HelicsPublication
    helicsFederateRegisterPublication(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);
HelicsPublication
    helicsFederateRegisterTypePublication(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);
HelicsPublication
    helicsFederateRegisterGlobalPublication(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);
HelicsPublication
    helicsFederateRegisterGlobalTypePublication(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);
HelicsInput helicsFederateRegisterInput(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);
HelicsInput helicsFederateRegisterTypeInput(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);
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
HelicsComplex helicsInputGetComplexObject(HelicsInput ipt, HelicsError* err);
void helicsInputGetComplex(HelicsInput ipt, double* real, double* imag, HelicsError* err);
int helicsInputGetVectorSize(HelicsInput ipt);
void helicsInputGetVector(HelicsInput ipt, double data[], int maxLength, int* actualSize, HelicsError* err);
void helicsInputGetNamedPoint(HelicsInput ipt, char* outputString, int maxStringLength, int* actualLength, double* val, HelicsError* err);

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
const char* helicsInputGetName(HelicsInput ipt);
const char* helicsSubscriptionGetTarget(HelicsInput ipt);
const char* helicsPublicationGetName(HelicsPublication pub);
const char* helicsInputGetUnits(HelicsInput ipt);
const char* helicsInputGetInjectionUnits(HelicsInput ipt);
const char* helicsInputGetExtractionUnits(HelicsInput ipt);
const char* helicsPublicationGetUnits(HelicsPublication pub);
const char* helicsInputGetInfo(HelicsInput inp);
void helicsInputSetInfo(HelicsInput inp, const char* info, HelicsError* err);
const char* helicsInputGetTag(HelicsInput inp, const char* tagname);
void helicsInputSetTag(HelicsInput inp, const char* tagname, const char* tagvalue, HelicsError* err);
const char* helicsPublicationGetInfo(HelicsPublication pub);
void helicsPublicationSetInfo(HelicsPublication pub, const char* info, HelicsError* err);
const char* helicsPublicationGetTag(HelicsPublication pub, const char* tagname);
void helicsPublicationSetTag(HelicsPublication pub, const char* tagname, const char* tagvalue, HelicsError* err);
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

HelicsEndpoint helicsFederateRegisterEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err);
HelicsEndpoint helicsFederateRegisterGlobalEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err);
HelicsEndpoint helicsFederateRegisterTargetedEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err);
HelicsEndpoint helicsFederateRegisterGlobalTargetedEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err);
HelicsEndpoint helicsFederateGetEndpoint(HelicsFederate fed, const char* name, HelicsError* err);
HelicsEndpoint helicsFederateGetEndpointByIndex(HelicsFederate fed, int index, HelicsError* err);
HelicsBool helicsEndpointIsValid(HelicsEndpoint endpoint);
void helicsEndpointSetDefaultDestination(HelicsEndpoint endpoint, const char* dst, HelicsError* err);
const char* helicsEndpointGetDefaultDestination(HelicsEndpoint endpoint);
void helicsEndpointSendBytes(HelicsEndpoint endpoint, const void* data, int inputDataLength, HelicsError* err);
void helicsEndpointSendBytesTo(HelicsEndpoint endpoint, const void* data, int inputDataLength, const char* dst, HelicsError* err);
void helicsEndpointSendBytesToAt(HelicsEndpoint endpoint,
                                 const void* data,
                                 int inputDataLength,
                                 const char* dst,
                                 HelicsTime time,
                                 HelicsError* err);
void helicsEndpointSendBytesAt(HelicsEndpoint endpoint, const void* data, int inputDataLength, HelicsTime time, HelicsError* err);
void helicsEndpointSendMessage(HelicsEndpoint endpoint, HelicsMessage message, HelicsError* err);
void helicsEndpointSendMessageZeroCopy(HelicsEndpoint endpoint, HelicsMessage message, HelicsError* err);
void helicsEndpointSubscribe(HelicsEndpoint endpoint, const char* key, HelicsError* err);
HelicsBool helicsFederateHasMessage(HelicsFederate fed);
HelicsBool helicsEndpointHasMessage(HelicsEndpoint endpoint);
int helicsFederatePendingMessageCount(HelicsFederate fed);
int helicsEndpointPendingMessageCount(HelicsEndpoint endpoint);
HelicsMessage helicsEndpointGetMessage(HelicsEndpoint endpoint);
HelicsMessage helicsEndpointCreateMessage(HelicsEndpoint endpoint, HelicsError* err);
HelicsMessage helicsFederateGetMessage(HelicsFederate fed);
HelicsMessage helicsFederateCreateMessage(HelicsFederate fed, HelicsError* err);
void helicsFederateClearMessages(HelicsFederate fed);
const char* helicsEndpointGetType(HelicsEndpoint endpoint);
const char* helicsEndpointGetName(HelicsEndpoint endpoint);
int helicsFederateGetEndpointCount(HelicsFederate fed);
const char* helicsEndpointGetInfo(HelicsEndpoint end);
void helicsEndpointSetInfo(HelicsEndpoint endpoint, const char* info, HelicsError* err);
const char* helicsEndpointGetTag(HelicsEndpoint endpoint, const char* tagname);
void helicsEndpointSetTag(HelicsEndpoint endpoint, const char* tagname, const char* tagvalue, HelicsError* err);
void helicsEndpointSetOption(HelicsEndpoint endpoint, int option, int value, HelicsError* err);
int helicsEndpointGetOption(HelicsEndpoint endpoint, int option);
void helicsEndpointAddSourceTarget(HelicsEndpoint endpoint, const char* targetEndpoint, HelicsError* err);
void helicsEndpointAddDestinationTarget(HelicsEndpoint endpoint, const char* targetEndpoint, HelicsError* err);
void helicsEndpointRemoveTarget(HelicsEndpoint endpoint, const char* targetEndpoint, HelicsError* err);
void helicsEndpointAddSourceFilter(HelicsEndpoint endpoint, const char* filterName, HelicsError* err);
void helicsEndpointAddDestinationFilter(HelicsEndpoint endpoint, const char* filterName, HelicsError* err);

const char* helicsMessageGetSource(HelicsMessage message);
const char* helicsMessageGetDestination(HelicsMessage message);
const char* helicsMessageGetOriginalSource(HelicsMessage message);
const char* helicsMessageGetOriginalDestination(HelicsMessage message);
HelicsTime helicsMessageGetTime(HelicsMessage message);
const char* helicsMessageGetString(HelicsMessage message);
int helicsMessageGetMessageID(HelicsMessage message);
HelicsBool helicsMessageGetFlagOption(HelicsMessage message, int flag);
int helicsMessageGetByteCount(HelicsMessage message);
void helicsMessageGetBytes(HelicsMessage message, void* data, int maxMessageLength, int* actualSize, HelicsError* err);
void* helicsMessageGetBytesPointer(HelicsMessage message);
HelicsBool helicsMessageIsValid(HelicsMessage message);
void helicsMessageSetSource(HelicsMessage message, const char* src, HelicsError* err);
void helicsMessageSetDestination(HelicsMessage message, const char* dst, HelicsError* err);
void helicsMessageSetOriginalSource(HelicsMessage message, const char* src, HelicsError* err);
void helicsMessageSetOriginalDestination(HelicsMessage message, const char* dst, HelicsError* err);
void helicsMessageSetTime(HelicsMessage message, HelicsTime time, HelicsError* err);
void helicsMessageResize(HelicsMessage message, int newSize, HelicsError* err);
void helicsMessageReserve(HelicsMessage message, int reserveSize, HelicsError* err);
void helicsMessageSetMessageID(HelicsMessage message, int32_t messageID, HelicsError* err);
void helicsMessageClearFlags(HelicsMessage message);
void helicsMessageSetFlagOption(HelicsMessage message, int flag, HelicsBool flagValue, HelicsError* err);
void helicsMessageSetString(HelicsMessage message, const char* str, HelicsError* err);
void helicsMessageSetData(HelicsMessage message, const void* data, int inputDataLength, HelicsError* err);
void helicsMessageAppendData(HelicsMessage message, const void* data, int inputDataLength, HelicsError* err);
void helicsMessageCopy(HelicsMessage src_message, HelicsMessage dst_message, HelicsError* err);
HelicsMessage helicsMessageClone(HelicsMessage message, HelicsError* err);
void helicsMessageFree(HelicsMessage message);
void helicsMessageClear(HelicsMessage message, HelicsError* err);

HelicsFilter helicsFederateRegisterFilter(HelicsFederate fed, HelicsFilterTypes type, const char* name, HelicsError* err);
HelicsFilter helicsFederateRegisterGlobalFilter(HelicsFederate fed, HelicsFilterTypes type, const char* name, HelicsError* err);
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
const char* helicsFilterGetTag(HelicsFilter filt, const char* tagname);
void helicsFilterSetTag(HelicsFilter filt, const char* tagname, const char* tagvalue, HelicsError* err);
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
void helicsFederateSetLoggingCallback(HelicsFederate fed,
                                      void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                      void* userdata,
                                      HelicsError* err);
void helicsFilterSetCustomCallback(HelicsFilter filter,
                                   void (*filtCall)(HelicsMessage message, void* userData),
                                   void* userdata,
                                   HelicsError* err);
void helicsFederateSetQueryCallback(HelicsFederate fed,
                                    void (*queryAnswer)(const char* query, int querySize, HelicsQueryBuffer buffer, void* userdata),
                                    void* userdata,
                                    HelicsError* err);
void helicsQueryBufferFill(HelicsQueryBuffer buffer, const char* str, int strSize, HelicsError* err);
