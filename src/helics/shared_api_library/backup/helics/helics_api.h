/*
Copyright (c) 2017-2024,
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
    HELICS_CORE_TYPE_NULL = 66,
    HELICS_CORE_TYPE_EMPTY = 77,
    HELICS_CORE_TYPE_EXTRACT = 101
} HelicsCoreTypes;

typedef enum {
    HELICS_DATA_TYPE_UNKNOWN = -1,
    HELICS_DATA_TYPE_STRING = 0,
    HELICS_DATA_TYPE_DOUBLE = 1,
    HELICS_DATA_TYPE_INT = 2,
    HELICS_DATA_TYPE_COMPLEX = 3,
    HELICS_DATA_TYPE_VECTOR = 4,
    HELICS_DATA_TYPE_COMPLEX_VECTOR = 5,
    HELICS_DATA_TYPE_NAMED_POINT = 6,
    HELICS_DATA_TYPE_BOOLEAN = 7,
    HELICS_DATA_TYPE_TIME = 8,
    HELICS_DATA_TYPE_CHAR = 9,
    HELICS_DATA_TYPE_RAW = 25,
    HELICS_DATA_TYPE_JSON = 30,
    HELICS_DATA_TYPE_MULTI = 33,
    HELICS_DATA_TYPE_ANY = 25262
} HelicsDataTypes;

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
    HELICS_FLAG_MULTI_THREAD_CORE = 28,
    HELICS_FLAG_SINGLE_THREAD_CORE = 29,
    HELICS_FLAG_REENTRANT = 38,
    HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS = 67,
    HELICS_FLAG_STRICT_CONFIG_CHECKING = 75,
    HELICS_FLAG_USE_JSON_SERIALIZATION = 79,
    HELICS_FLAG_EVENT_TRIGGERED = 81,
    HELICS_FLAG_LOCAL_PROFILING_CAPTURE = 96,
    HELICS_FLAG_CALLBACK_FEDERATE = 103,
    HELICS_FLAG_AUTOMATED_TIME_REQUEST = 106
} HelicsFederateFlags;

typedef enum {

    HELICS_FLAG_DELAY_INIT_ENTRY = 45,
    HELICS_FLAG_ENABLE_INIT_ENTRY = 47,
    HELICS_FLAG_IGNORE = 999
} HelicsCoreFlags;

typedef enum {

    HELICS_FLAG_SLOW_RESPONDING = 29,
    HELICS_FLAG_DEBUGGING = 31,
    HELICS_FLAG_TERMINATE_ON_ERROR = 72,
    HELICS_FLAG_FORCE_LOGGING_FLUSH = 88,
    HELICS_FLAG_DUMPLOG = 89,
    HELICS_FLAG_PROFILING = 93,
    HELICS_FLAG_PROFILING_MARKER = 95,
    HELICS_FLAG_ALLOW_REMOTE_CONTROL = 109,
    HELICS_FLAG_DISABLE_REMOTE_CONTROL = 110
} HelicsFlags;

typedef enum {

    HELICS_LOG_LEVEL_DUMPLOG = -10,
    HELICS_LOG_LEVEL_NO_PRINT = -4,
    HELICS_LOG_LEVEL_ERROR = 0,
    HELICS_LOG_LEVEL_PROFILING = 2,
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
    HELICS_USER_EXCEPTION = -29,
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
    HELICS_OK = 0,
    HELICS_ERROR_USER_ABORT = 130,
    HELICS_ERROR_TERMINATED = 143
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
    HELICS_PROPERTY_TIME_STOPTIME = 152,
    HELICS_PROPERTY_TIME_GRANT_TIMEOUT = 161,
    HELICS_PROPERTY_INT_CURRENT_ITERATION = 258,
    HELICS_PROPERTY_INT_MAX_ITERATIONS = 259,
    HELICS_PROPERTY_INT_LOG_LEVEL = 271,
    HELICS_PROPERTY_INT_FILE_LOG_LEVEL = 272,
    HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL = 274,
    HELICS_PROPERTY_INT_LOG_BUFFER = 276,
    HELICS_PROPERTY_INT_INDEX_GROUP = 282
} HelicsProperties;

const int HELICS_INVALID_PROPERTY_VALUE = -972;

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
    HELICS_HANDLE_OPTION_RECONNECTABLE = 412,
    HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING = 414,
    HELICS_HANDLE_OPTION_RECEIVE_ONLY = 422,
    HELICS_HANDLE_OPTION_SOURCE_ONLY = 423,
    HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH = 447,
    HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE = 452,
    HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE = 454,
    HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS = 475,
    HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD = 507,
    HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION = 510,
    HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST = 512,
    HELICS_HANDLE_OPTION_CONNECTIONS = 522,
    HELICS_HANDLE_OPTION_TIME_RESTRICTED = 557
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

typedef enum {

    HELICS_TRANSLATOR_TYPE_CUSTOM = 0,
    HELICS_TRANSLATOR_TYPE_JSON = 11,
    HELICS_TRANSLATOR_TYPE_BINARY = 12
} HelicsTranslatorTypes;

typedef enum {

    HELICS_SEQUENCING_MODE_FAST = 0,
    HELICS_SEQUENCING_MODE_ORDERED = 1,
    HELICS_SEQUENCING_MODE_DEFAULT = 2
} HelicsSequencingModes;

#define HELICS_BIG_NUMBER 9223372036.854774
const double cHelicsBigNumber = HELICS_BIG_NUMBER;

#define HELICS_INVALID_DOUBLE (-1E49)
typedef void* HelicsInput;

typedef void* HelicsPublication;

typedef void* HelicsEndpoint;

typedef void* HelicsFilter;

typedef void* HelicsTranslator;

typedef void* HelicsCore;

typedef void* HelicsBroker;

typedef void* HelicsFederate;

typedef void* HelicsApp;

typedef void* HelicsFederateInfo;

typedef void* HelicsQuery;

typedef void* HelicsDataBuffer;

typedef void* HelicsQueryBuffer;

typedef void* HelicsMessage;

typedef double HelicsTime;

const HelicsTime HELICS_TIME_ZERO = 0.0;
const HelicsTime HELICS_TIME_EPSILON = 1.0e-9;
const HelicsTime HELICS_TIME_INVALID = -1.785e39;
const HelicsTime HELICS_TIME_MAXTIME = HELICS_BIG_NUMBER;

typedef int HelicsBool;

const HelicsBool HELICS_TRUE = 1;
const HelicsBool HELICS_FALSE = 0;

typedef enum {
    HELICS_ITERATION_REQUEST_NO_ITERATION = 0,
    HELICS_ITERATION_REQUEST_FORCE_ITERATION = 1,
    HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED = 2,
    HELICS_ITERATION_REQUEST_HALT_OPERATIONS = 5,
    HELICS_ITERATION_REQUEST_ERROR = 7
} HelicsIterationRequest;

typedef enum {
    HELICS_ITERATION_RESULT_NEXT_STEP = 0,
    HELICS_ITERATION_RESULT_ERROR = 1,
    HELICS_ITERATION_RESULT_HALTED = 2,
    HELICS_ITERATION_RESULT_ITERATING = 3
} HelicsIterationResult;

typedef enum {
    HELICS_STATE_UNKNOWN = -1,
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

HelicsDataBuffer helicsCreateDataBuffer(int32_t initialCapacity);
HelicsBool helicsDataBufferIsValid(HelicsDataBuffer data);
HelicsDataBuffer helicsWrapDataInBuffer(void* data, int dataSize, int dataCapacity);
void helicsDataBufferFree(HelicsDataBuffer data);
int32_t helicsDataBufferSize(HelicsDataBuffer data);
int32_t helicsDataBufferCapacity(HelicsDataBuffer data);
void* helicsDataBufferData(HelicsDataBuffer data);
HelicsBool helicsDataBufferReserve(HelicsDataBuffer data, int32_t newCapacity);
HelicsDataBuffer helicsDataBufferClone(HelicsDataBuffer data);
int32_t helicsDataBufferFillFromInteger(HelicsDataBuffer data, int64_t value);
int32_t helicsDataBufferFillFromDouble(HelicsDataBuffer data, double value);
int32_t helicsDataBufferFillFromString(HelicsDataBuffer data, const char* value);
int32_t helicsDataBufferFillFromRawString(HelicsDataBuffer data, const char* str, int stringSize);
int32_t helicsDataBufferFillFromBoolean(HelicsDataBuffer data, HelicsBool value);
int32_t helicsDataBufferFillFromChar(HelicsDataBuffer data, char value);
int32_t helicsDataBufferFillFromTime(HelicsDataBuffer data, HelicsTime value);
int32_t helicsDataBufferFillFromComplex(HelicsDataBuffer data, double real, double imag);
int32_t helicsDataBufferFillFromComplexObject(HelicsDataBuffer data, HelicsComplex value);
int32_t helicsDataBufferFillFromVector(HelicsDataBuffer data, const double* value, int dataSize);
int32_t helicsDataBufferFillFromNamedPoint(HelicsDataBuffer data, const char* name, double value);
int32_t helicsDataBufferFillFromComplexVector(HelicsDataBuffer data, const double* value, int dataSize);
int helicsDataBufferType(HelicsDataBuffer data);
int64_t helicsDataBufferToInteger(HelicsDataBuffer data);
double helicsDataBufferToDouble(HelicsDataBuffer data);
HelicsBool helicsDataBufferToBoolean(HelicsDataBuffer data);
char helicsDataBufferToChar(HelicsDataBuffer data);
int helicsDataBufferStringSize(HelicsDataBuffer data);
void helicsDataBufferToString(HelicsDataBuffer data, char* outputString, int maxStringLen, int* actualLength);
void helicsDataBufferToRawString(HelicsDataBuffer data, char* outputString, int maxStringLen, int* actualLength);
HelicsTime helicsDataBufferToTime(HelicsDataBuffer data);
HelicsComplex helicsDataBufferToComplexObject(HelicsDataBuffer data);
void helicsDataBufferToComplex(HelicsDataBuffer data, double* real, double* imag);
int helicsDataBufferVectorSize(HelicsDataBuffer data);
void helicsDataBufferToVector(HelicsDataBuffer data, double values[], int maxlen, int* actualSize);
void helicsDataBufferToComplexVector(HelicsDataBuffer data, double values[], int maxlen, int* actualSize);
void helicsDataBufferToNamedPoint(HelicsDataBuffer data, char* outputString, int maxStringLength, int* actualLength, double* val);
HelicsBool helicsDataBufferConvertToType(HelicsDataBuffer data, int newDataType);
const char* helicsGetVersion(void);
const char* helicsGetBuildFlags(void);
const char* helicsGetCompilerVersion(void);
const char* helicsGetSystemInfo(void);
HelicsError helicsErrorInitialize(void);
void helicsErrorClear(HelicsError* err);
void helicsLoadSignalHandler();
void helicsLoadThreadedSignalHandler();
void helicsClearSignalHandler();
void helicsLoadSignalHandlerCallback(HelicsBool (*handler)(int), HelicsBool useSeparateThread);
void helicsLoadSignalHandlerCallbackNoExit(HelicsBool (*handler)(int), HelicsBool useSeparateThread);
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
HelicsFederate helicsCreateValueFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err);
HelicsFederate helicsCreateValueFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsCreateMessageFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err);
HelicsFederate helicsCreateMessageFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsCreateCombinationFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err);
HelicsFederate helicsCreateCombinationFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsCreateCallbackFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err);
HelicsFederate helicsCreateCallbackFederateFromConfig(const char* configFile, HelicsError* err);
HelicsFederate helicsFederateClone(HelicsFederate fed, HelicsError* err);
void helicsFederateProtect(const char* fedName, HelicsError* err);
void helicsFederateUnProtect(const char* fedName, HelicsError* err);
HelicsBool helicsFederateIsProtected(const char* fedName, HelicsError* err);
HelicsFederateInfo helicsCreateFederateInfo(void);
HelicsFederateInfo helicsFederateInfoClone(HelicsFederateInfo fedInfo, HelicsError* err);
void helicsFederateInfoLoadFromArgs(HelicsFederateInfo fedInfo, int argc, const char* const* argv, HelicsError* err);
void helicsFederateInfoLoadFromString(HelicsFederateInfo fedInfo, const char* args, HelicsError* err);
void helicsFederateInfoFree(HelicsFederateInfo fedInfo);
HelicsBool helicsFederateIsValid(HelicsFederate fed);
void helicsFederateInfoSetCoreName(HelicsFederateInfo fedInfo, const char* corename, HelicsError* err);
void helicsFederateInfoSetCoreInitString(HelicsFederateInfo fedInfo, const char* coreInit, HelicsError* err);
void helicsFederateInfoSetBrokerInitString(HelicsFederateInfo fedInfo, const char* brokerInit, HelicsError* err);
void helicsFederateInfoSetCoreType(HelicsFederateInfo fedInfo, int coretype, HelicsError* err);
void helicsFederateInfoSetCoreTypeFromString(HelicsFederateInfo fedInfo, const char* coretype, HelicsError* err);
void helicsFederateInfoSetBroker(HelicsFederateInfo fedInfo, const char* broker, HelicsError* err);
void helicsFederateInfoSetBrokerKey(HelicsFederateInfo fedInfo, const char* brokerkey, HelicsError* err);
void helicsFederateInfoSetBrokerPort(HelicsFederateInfo fedInfo, int brokerPort, HelicsError* err);
void helicsFederateInfoSetLocalPort(HelicsFederateInfo fedInfo, const char* localPort, HelicsError* err);
int helicsGetPropertyIndex(const char* val);
int helicsGetFlagIndex(const char* val);
int helicsGetOptionIndex(const char* val);
int helicsGetOptionValue(const char* val);
int helicsGetDataType(const char* val);
void helicsFederateInfoSetFlagOption(HelicsFederateInfo fedInfo, int flag, HelicsBool value, HelicsError* err);
void helicsFederateInfoSetSeparator(HelicsFederateInfo fedInfo, char separator, HelicsError* err);
void helicsFederateInfoSetTimeProperty(HelicsFederateInfo fedInfo, int timeProperty, HelicsTime propertyValue, HelicsError* err);
void helicsFederateInfoSetIntegerProperty(HelicsFederateInfo fedInfo, int intProperty, int propertyValue, HelicsError* err);
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
void helicsFederateEnterInitializingModeComplete(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterInitializingModeIterative(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterInitializingModeIterativeAsync(HelicsFederate fed, HelicsError* err);
void helicsFederateEnterInitializingModeIterativeComplete(HelicsFederate fed, HelicsError* err);
HelicsBool helicsFederateIsAsyncOperationCompleted(HelicsFederate fed, HelicsError* err);
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
void helicsFederateProcessCommunications(HelicsFederate fed, HelicsTime period, HelicsError* err);
const char* helicsFederateGetName(HelicsFederate fed);
void helicsFederateSetTimeProperty(HelicsFederate fed, int timeProperty, HelicsTime time, HelicsError* err);
void helicsFederateSetFlagOption(HelicsFederate fed, int flag, HelicsBool flagValue, HelicsError* err);
void helicsFederateSetSeparator(HelicsFederate fed, char separator, HelicsError* err);
void helicsFederateSetIntegerProperty(HelicsFederate fed, int intProperty, int propertyVal, HelicsError* err);
HelicsTime helicsFederateGetTimeProperty(HelicsFederate fed, int timeProperty, HelicsError* err);
HelicsBool helicsFederateGetFlagOption(HelicsFederate fed, int flag, HelicsError* err);
int helicsFederateGetIntegerProperty(HelicsFederate fed, int intProperty, HelicsError* err);
HelicsTime helicsFederateGetCurrentTime(HelicsFederate fed, HelicsError* err);
void helicsFederateAddAlias(HelicsFederate fed, const char* interfaceName, const char* alias, HelicsError* err);
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
void helicsCoreAddAlias(HelicsCore core, const char* interfaceName, const char* alias, HelicsError* err);
void helicsBrokerAddAlias(HelicsBroker broker, const char* interfaceName, const char* alias, HelicsError* err);
void helicsCoreSendCommand(HelicsCore core, const char* target, const char* command, HelicsError* err);
void helicsCoreSendOrderedCommand(HelicsCore core, const char* target, const char* command, HelicsError* err);
void helicsBrokerSendCommand(HelicsBroker broker, const char* target, const char* command, HelicsError* err);
void helicsBrokerSendOrderedCommand(HelicsBroker broker, const char* target, const char* command, HelicsError* err);
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
HelicsBool helicsAppEnabled();
HelicsApp helicsCreateApp(const char* appName, const char* appType, const char* configFile, HelicsFederateInfo fedInfo, HelicsError* err);
HelicsFederate helicsAppGetFederate(HelicsApp app, HelicsError* err);
void helicsAppLoadFile(HelicsApp app, const char* configFile, HelicsError* err);
void helicsAppInitialize(HelicsApp app, HelicsError* err);
void helicsAppRun(HelicsApp app, HelicsError* err);
void helicsAppRunTo(HelicsApp app, HelicsTime stopTime, HelicsError* err);
void helicsAppFinalize(HelicsApp app, HelicsError* err);
void helicsAppFree(HelicsApp app);
void helicsAppDestroy(HelicsApp app);
HelicsBool helicsAppIsActive(HelicsApp app);
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
HELICS_DEPRECATED HelicsInput helicsFederateGetSubscription(HelicsFederate fed, const char* key, HelicsError* err);
HelicsInput helicsFederateGetInputByTarget(HelicsFederate fed, const char* target, HelicsError* err);
void helicsFederateClearUpdates(HelicsFederate fed);
void helicsFederateRegisterFromPublicationJSON(HelicsFederate fed, const char* json, HelicsError* err);
void helicsFederatePublishJSON(HelicsFederate fed, const char* json, HelicsError* err);
HelicsBool helicsPublicationIsValid(HelicsPublication pub);
void helicsPublicationPublishBytes(HelicsPublication pub, const void* data, int inputDataLength, HelicsError* err);
void helicsPublicationPublishString(HelicsPublication pub, const char* val, HelicsError* err);
void helicsPublicationPublishInteger(HelicsPublication pub, int64_t val, HelicsError* err);
void helicsPublicationPublishBoolean(HelicsPublication pub, HelicsBool val, HelicsError* err);
void helicsPublicationPublishDouble(HelicsPublication pub, double val, HelicsError* err);
void helicsPublicationPublishTime(HelicsPublication pub, HelicsTime val, HelicsError* err);
void helicsPublicationPublishChar(HelicsPublication pub, char val, HelicsError* err);
void helicsPublicationPublishComplex(HelicsPublication pub, double real, double imag, HelicsError* err);
void helicsPublicationPublishVector(HelicsPublication pub, const double* vectorInput, int vectorLength, HelicsError* err);
void helicsPublicationPublishComplexVector(HelicsPublication pub, const double* vectorInput, int vectorLength, HelicsError* err);
void helicsPublicationPublishNamedPoint(HelicsPublication pub, const char* field, double val, HelicsError* err);
void helicsPublicationPublishDataBuffer(HelicsPublication pub, HelicsDataBuffer buffer, HelicsError* err);
void helicsPublicationAddTarget(HelicsPublication pub, const char* target, HelicsError* err);
HelicsBool helicsInputIsValid(HelicsInput ipt);
void helicsInputAddTarget(HelicsInput ipt, const char* target, HelicsError* err);

int helicsInputGetByteCount(HelicsInput ipt);
void helicsInputGetBytes(HelicsInput ipt, void* data, int maxDataLength, int* actualSize, HelicsError* err);
HelicsDataBuffer helicsInputGetDataBuffer(HelicsInput inp, HelicsError* err);
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
void helicsInputGetComplexVector(HelicsInput ipt, double data[], int maxLength, int* actualSize, HelicsError* err);
void helicsInputGetNamedPoint(HelicsInput ipt, char* outputString, int maxStringLength, int* actualLength, double* val, HelicsError* err);

void helicsInputSetDefaultBytes(HelicsInput ipt, const void* data, int inputDataLength, HelicsError* err);
void helicsInputSetDefaultString(HelicsInput ipt, const char* defaultString, HelicsError* err);
void helicsInputSetDefaultInteger(HelicsInput ipt, int64_t val, HelicsError* err);
void helicsInputSetDefaultBoolean(HelicsInput ipt, HelicsBool val, HelicsError* err);
void helicsInputSetDefaultTime(HelicsInput ipt, HelicsTime val, HelicsError* err);
void helicsInputSetDefaultChar(HelicsInput ipt, char val, HelicsError* err);
void helicsInputSetDefaultDouble(HelicsInput ipt, double val, HelicsError* err);
void helicsInputSetDefaultComplex(HelicsInput ipt, double real, double imag, HelicsError* err);
void helicsInputSetDefaultVector(HelicsInput ipt, const double* vectorInput, int vectorLength, HelicsError* err);
void helicsInputSetDefaultComplexVector(HelicsInput ipt, const double* vectorInput, int vectorLength, HelicsError* err);
void helicsInputSetDefaultNamedPoint(HelicsInput ipt, const char* defaultName, double val, HelicsError* err);

const char* helicsInputGetType(HelicsInput ipt);
const char* helicsInputGetPublicationType(HelicsInput ipt);
int helicsInputGetPublicationDataType(HelicsInput ipt);
const char* helicsPublicationGetType(HelicsPublication pub);
const char* helicsInputGetName(HelicsInput ipt);
HELICS_DEPRECATED const char* helicsSubscriptionGetTarget(HelicsInput ipt);
const char* helicsInputGetTarget(HelicsInput ipt);
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
void helicsEndpointSendString(HelicsEndpoint endpoint, const char* message, HelicsError* err);
void helicsEndpointSendStringTo(HelicsEndpoint endpoint, const char* message, const char* dst, HelicsError* err);
void helicsEndpointSendStringToAt(HelicsEndpoint endpoint, const char* message, const char* dst, HelicsTime time, HelicsError* err);
void helicsEndpointSendStringAt(HelicsEndpoint endpoint, const char* message, HelicsTime time, HelicsError* err);
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
void helicsEndpointClearMessages(HelicsEndpoint endpoint);
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
HelicsDataBuffer helicsMessageDataBuffer(HelicsMessage message, HelicsError* err);
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
void helicsMessageSetString(HelicsMessage message, const char* data, HelicsError* err);
void helicsMessageSetData(HelicsMessage message, const void* data, int inputDataLength, HelicsError* err);
void helicsMessageSetDataBuffer(HelicsMessage message, HelicsDataBuffer data, HelicsError* err);
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
double helicsFilterGetPropertyDouble(HelicsFilter filt, const char* prop);
const char* helicsFilterGetPropertyString(HelicsFilter filt, const char* prop);
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

HelicsTranslator helicsFederateRegisterTranslator(HelicsFederate fed, HelicsTranslatorTypes type, const char* name, HelicsError* err);
HelicsTranslator helicsFederateRegisterGlobalTranslator(HelicsFederate fed, HelicsTranslatorTypes type, const char* name, HelicsError* err);
HelicsTranslator helicsCoreRegisterTranslator(HelicsCore core, HelicsTranslatorTypes type, const char* name, HelicsError* err);
int helicsFederateGetTranslatorCount(HelicsFederate fed);
HelicsTranslator helicsFederateGetTranslator(HelicsFederate fed, const char* name, HelicsError* err);
HelicsTranslator helicsFederateGetTranslatorByIndex(HelicsFederate fed, int index, HelicsError* err);
HelicsBool helicsTranslatorIsValid(HelicsTranslator trans);
const char* helicsTranslatorGetName(HelicsTranslator trans);
void helicsTranslatorSet(HelicsTranslator trans, const char* prop, double val, HelicsError* err);
void helicsTranslatorSetString(HelicsTranslator trans, const char* prop, const char* val, HelicsError* err);
void helicsTranslatorAddInputTarget(HelicsTranslator trans, const char* input, HelicsError* err);
void helicsTranslatorAddPublicationTarget(HelicsTranslator trans, const char* pub, HelicsError* err);
void helicsTranslatorAddSourceEndpoint(HelicsTranslator trans, const char* ept, HelicsError* err);
void helicsTranslatorAddDestinationEndpoint(HelicsTranslator trans, const char* ept, HelicsError* err);
void helicsTranslatorRemoveTarget(HelicsTranslator trans, const char* target, HelicsError* err);
const char* helicsTranslatorGetInfo(HelicsTranslator trans);
void helicsTranslatorSetInfo(HelicsTranslator trans, const char* info, HelicsError* err);
const char* helicsTranslatorGetTag(HelicsTranslator trans, const char* tagname);
void helicsTranslatorSetTag(HelicsTranslator trans, const char* tagname, const char* tagvalue, HelicsError* err);
void helicsTranslatorSetOption(HelicsTranslator trans, int option, int value, HelicsError* err);
int helicsTranslatorGetOption(HelicsTranslator trans, int option);

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
                                   HelicsMessage (*filtCall)(HelicsMessage message, void* userData),
                                   void* userdata,
                                   HelicsError* err);
void helicsTranslatorSetCustomCallback(HelicsTranslator translator,
                                       void (*toMessageCall)(HelicsDataBuffer value, HelicsMessage message, void* userData),
                                       void (*toValueCall)(HelicsMessage message, HelicsDataBuffer value, void* userData),
                                       void* userdata,
                                       HelicsError* err);
void helicsFederateSetQueryCallback(HelicsFederate fed,
                                    void (*queryAnswer)(const char* query, int querySize, HelicsQueryBuffer buffer, void* userdata),
                                    void* userdata,
                                    HelicsError* err);
void helicsFederateSetTimeRequestEntryCallback(
    HelicsFederate fed,
    void (*requestTime)(HelicsTime currentTime, HelicsTime requestTime, HelicsBool iterating, void* userdata),
    void* userdata,
    HelicsError* err);
void helicsFederateSetTimeUpdateCallback(HelicsFederate fed,
                                         void (*timeUpdate)(HelicsTime newTime, HelicsBool iterating, void* userdata),
                                         void* userdata,
                                         HelicsError* err);
void helicsFederateSetStateChangeCallback(HelicsFederate fed,
                                          void (*stateChange)(HelicsFederateState newState, HelicsFederateState oldState, void* userdata),
                                          void* userdata,
                                          HelicsError* err);
void helicsFederateSetTimeRequestReturnCallback(HelicsFederate fed,
                                                void (*requestTimeReturn)(HelicsTime newTime, HelicsBool iterating, void* userdata),
                                                void* userdata,
                                                HelicsError* err);
void helicsFederateInitializingEntryCallback(HelicsFederate fed,
                                             void (*initializingEntry)(HelicsBool iterating, void* userdata),
                                             void* userdata,
                                             HelicsError* err);
void helicsFederateExecutingEntryCallback(HelicsFederate fed, void (*executingEntry)(void* userdata), void* userdata, HelicsError* err);
void helicsFederateCosimulationTerminationCallback(HelicsFederate fed,
                                                   void (*cosimTermination)(void* userdata),
                                                   void* userdata,
                                                   HelicsError* err);
void helicsFederateErrorHandlerCallback(HelicsFederate fed,
                                        void (*errorHandler)(int errorCode, const char* errorString, void* userdata),
                                        void* userdata,
                                        HelicsError* err);
void helicsCallbackFederateNextTimeCallback(HelicsFederate fed,
                                            HelicsTime (*timeUpdate)(HelicsTime time, void* userdata),
                                            void* userdata,
                                            HelicsError* err);
void helicsCallbackFederateNextTimeIterativeCallback(
    HelicsFederate fed,
    HelicsTime (*timeUpdate)(HelicsTime time, HelicsIterationResult result, HelicsIterationRequest* iteration, void* userdata),
    void* userdata,
    HelicsError* err);
void helicsCallbackFederateInitializeCallback(HelicsFederate fed,
                                              HelicsIterationRequest (*initialize)(void* userdata),
                                              void* userdata,
                                              HelicsError* err);
void helicsQueryBufferFill(HelicsQueryBuffer buffer, const char* queryResult, int strSize, HelicsError* err);
