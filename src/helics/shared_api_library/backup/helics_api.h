/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <stdint.h>
#include <stdlib.h>

typedef enum {
    helics_core_type_default = 0,
    helics_core_type_zmq = 1,
    helics_core_type_mpi = 2,
    helics_core_type_test = 3,

    helics_core_type_interprocess = 4,
    helics_core_type_ipc = 5,
    helics_core_type_tcp = 6,
    helics_core_type_udp = 7,
    helics_core_type_zmq_test = 10,
    helics_core_type_nng = 9,
    helics_core_type_tcp_ss = 11,
    helics_core_type_http = 12,
    helics_core_type_websocket = 14,
    helics_core_type_inproc = 18,
    helics_core_type_null = 66
} helics_core_type;

typedef enum {
    helics_data_type_string = 0,
    helics_data_type_double = 1,
    helics_data_type_int = 2,
    helics_data_type_complex = 3,
    helics_data_type_vector = 4,
    helics_data_type_complex_vector = 5,
    helics_data_type_named_point = 6,
    helics_data_type_boolean = 7,
    helics_data_type_time = 8,
    helics_data_type_raw = 25,
    helics_data_type_multi = 33,
    helics_data_type_any = 25262
} helics_data_type;

#define helics_data_type_char helics_data_type_string

typedef enum {
    helics_flag_observer = 0,
    helics_flag_uninterruptible = 1,
    helics_flag_interruptible = 2,
    helics_flag_source_only = 4,
    helics_flag_only_transmit_on_change = 6,
    helics_flag_only_update_on_change = 8,
    helics_flag_wait_for_current_time_update = 10,
    helics_flag_restrictive_time_policy = 11,
    helics_flag_rollback = 12,
    helics_flag_forward_compute = 14,
    helics_flag_realtime = 16,
    helics_flag_single_thread_federate = 27,
    helics_flag_ignore_time_mismatch_warnings = 67,
    helics_flag_strict_config_checking = 75,
} helics_federate_flags;

typedef enum {
    helics_flag_delay_init_entry = 45,
    helics_flag_enable_init_entry = 47,
} helics_core_flags;

typedef enum {
    helics_flag_slow_responding = 29,
    helics_flag_debugging = 31,
    helics_flag_terminate_on_error = 72,
    helics_flag_force_logging_flush = 88,
    helics_flag_dumplog = 89
} helics_flags;

typedef enum {
    helics_log_level_no_print = -1,
    helics_log_level_error = 0,
    helics_log_level_warning = 1,
    helics_log_level_summary = 2,
    helics_log_level_connections = 3,
    helics_log_level_interfaces = 4,
    helics_log_level_timing = 5,
    helics_log_level_data = 6,
    helics_log_level_trace = 7
} helics_log_levels;

typedef enum {
    helics_error_fatal = -404,
    helics_error_external_type = -203,
    helics_error_other = -101,
    helics_error_insufficient_space = -18,
    helics_error_execution_failure = -14,
    helics_error_invalid_function_call = -10,
    helics_error_invalid_state_transition = -9,
    helics_warning = -8,
    helics_error_system_failure = -6,
    helics_error_discard = -5,
    helics_error_invalid_argument = -4,
    helics_error_invalid_object = -3,
    helics_error_connection_failure = -2,
    helics_error_registration_failure = -1,
    helics_ok = 0
} helics_error_types;

typedef enum {
    helics_property_time_delta = 137,
    helics_property_time_period = 140,
    helics_property_time_offset = 141,
    helics_property_time_rt_lag = 143,
    helics_property_time_rt_lead = 144,
    helics_property_time_rt_tolerance = 145,
    helics_property_time_input_delay = 148,
    helics_property_time_output_delay = 150,
    helics_property_int_max_iterations = 259,
    helics_property_int_log_level = 271,
    helics_property_int_file_log_level = 272,
    helics_property_int_console_log_level = 274
} helics_properties;

typedef enum {
    helics_multi_input_no_op = 0,
    helics_multi_input_vectorize_operation = 1,
    helics_multi_input_and_operation = 2,
    helics_multi_input_or_operation = 3,
    helics_multi_input_sum_operation = 4,
    helics_multi_input_diff_operation = 5,
    helics_multi_input_max_operation = 6,
    helics_multi_input_min_operation = 7,
    helics_multi_input_average_operation = 8
} helics_multi_input_mode;

typedef enum {
    helics_handle_option_connection_required = 397,
    helics_handle_option_connection_optional = 402,
    helics_handle_option_single_connection_only = 407,
    helics_handle_option_multiple_connections_allowed = 409,
    helics_handle_option_buffer_data = 411,
    helics_handle_option_strict_type_checking = 414,
    helics_handle_option_ignore_unit_mismatch = 447,
    helics_handle_option_only_transmit_on_change = 452,
    helics_handle_option_only_update_on_change = 454,
    helics_handle_option_ignore_interrupts = 475,
    helics_handle_option_multi_input_handling_method = 507,
    helics_handle_option_input_priority_location = 510,
    helics_handle_option_clear_priority_list = 512,
    helics_handle_option_connections = 522
} helics_handle_options;

typedef enum {
    helics_filter_type_custom = 0,
    helics_filter_type_delay = 1,
    helics_filter_type_random_delay = 2,
    helics_filter_type_random_drop = 3,
    helics_filter_type_reroute = 4,
    helics_filter_type_clone = 5,
    helics_filter_type_firewall = 6
} helics_filter_type;

typedef void* helics_input;
typedef void* HelicsInput;

typedef void* helics_publication;
typedef void* HelicsPublication;

typedef void* helics_endpoint;
typedef void* HelicsEndpoint;

typedef void* helics_filter;
typedef void* HelicsFilter;

typedef void* helics_core;
typedef void* HelicsCore;

typedef void* helics_broker;
typedef void* HelicsBroker;

typedef void* helics_federate;
typedef void* HelicsFederate;

typedef void* helics_federate_info;
typedef void* HelicsFederateInfo;

typedef void* helics_query;
typedef void* HelicsQuery;

typedef void* helics_query_buffer;

typedef void* helics_message;
typedef void* HelicsMessage;

typedef double helics_time;
typedef double HelicsTime;

const HelicsTime helics_time_zero = 0.0;
const HelicsTime helics_time_epsilon = 1.0e-9;
const HelicsTime helics_time_invalid = -1.785e39;
const HelicsTime helics_time_maxtime = 9223372036.854774;

typedef int helics_bool;
typedef int HelicsBool;

const helics_bool helics_true = 1;
const helics_bool helics_false = 0;

typedef enum {
    helics_iteration_request_no_iteration,
    helics_iteration_request_force_iteration,
    helics_iteration_request_iterate_if_needed
} helics_iteration_request;

typedef enum {
    helics_iteration_result_next_step,
    helics_iteration_result_error,
    helics_iteration_result_halted,
    helics_iteration_result_iterating
} helics_iteration_result;

typedef enum {
    helics_state_startup = 0,
    helics_state_initialization,
    helics_state_execution,
    helics_state_finalize,
    helics_state_error,

    helics_state_pending_init,
    helics_state_pending_exec,
    helics_state_pending_time,
    helics_state_pending_iterative_time,
    helics_state_pending_finalize
} helics_federate_state;

typedef struct HelicsComplex {
    double real;
    double imag;
} HelicsComplex;

typedef HelicsComplex helics_complex;

typedef struct helics_error {
    int32_t error_code;
    const char* message;
} helics_error;

typedef helics_error HelicsError;

const char* helicsGetVersion(void);
const char* helicsGetBuildFlags(void);
const char* helicsGetCompilerVersion(void);
helics_error helicsErrorInitialize(void);
void helicsErrorClear(helics_error* err);
helics_bool helicsIsCoreTypeAvailable(const char* type);
helics_core helicsCreateCore(const char* type, const char* name, const char* initString, helics_error* err);
helics_core helicsCreateCoreFromArgs(const char* type, const char* name, int argc, const char* const* argv, helics_error* err);
helics_core helicsCoreClone(helics_core core, helics_error* err);
helics_bool helicsCoreIsValid(helics_core core);
helics_broker helicsCreateBroker(const char* type, const char* name, const char* initString, helics_error* err);
helics_broker helicsCreateBrokerFromArgs(const char* type, const char* name, int argc, const char* const* argv, helics_error* err);
helics_broker helicsBrokerClone(helics_broker broker, helics_error* err);
helics_bool helicsBrokerIsValid(helics_broker broker);
helics_bool helicsBrokerIsConnected(helics_broker broker);
void helicsBrokerDataLink(helics_broker broker, const char* source, const char* target, helics_error* err);
void helicsBrokerAddSourceFilterToEndpoint(helics_broker broker, const char* filter, const char* endpoint, helics_error* err);
void helicsBrokerAddDestinationFilterToEndpoint(helics_broker broker, const char* filter, const char* endpoint, helics_error* err);
void helicsBrokerMakeConnections(helics_broker broker, const char* file, helics_error* err);
helics_bool helicsCoreWaitForDisconnect(helics_core core, int msToWait, helics_error* err);
helics_bool helicsBrokerWaitForDisconnect(helics_broker broker, int msToWait, helics_error* err);
helics_bool helicsCoreIsConnected(helics_core core);
void helicsCoreDataLink(helics_core core, const char* source, const char* target, helics_error* err);
void helicsCoreAddSourceFilterToEndpoint(helics_core core, const char* filter, const char* endpoint, helics_error* err);
void helicsCoreAddDestinationFilterToEndpoint(helics_core core, const char* filter, const char* endpoint, helics_error* err);
void helicsCoreMakeConnections(helics_core core, const char* file, helics_error* err);
const char* helicsBrokerGetIdentifier(helics_broker broker);
const char* helicsCoreGetIdentifier(helics_core core);
const char* helicsBrokerGetAddress(helics_broker broker);
const char* helicsCoreGetAddress(helics_core core);
void helicsCoreSetReadyToInit(helics_core core, helics_error* err);
helics_bool helicsCoreConnect(helics_core core, helics_error* err);
void helicsCoreDisconnect(helics_core core, helics_error* err);
helics_federate helicsGetFederateByName(const char* fedName, helics_error* err);
void helicsBrokerDisconnect(helics_broker broker, helics_error* err);
void helicsFederateDestroy(helics_federate fed);
void helicsBrokerDestroy(helics_broker broker);
void helicsCoreDestroy(helics_core core);
void helicsCoreFree(helics_core core);
void helicsBrokerFree(helics_broker broker);

helics_federate helicsCreateValueFederate(const char* fedName, helics_federate_info fi, helics_error* err);
helics_federate helicsCreateValueFederateFromConfig(const char* configFile, helics_error* err);
helics_federate helicsCreateMessageFederate(const char* fedName, helics_federate_info fi, helics_error* err);
helics_federate helicsCreateMessageFederateFromConfig(const char* configFile, helics_error* err);
helics_federate helicsCreateCombinationFederate(const char* fedName, helics_federate_info fi, helics_error* err);
helics_federate helicsCreateCombinationFederateFromConfig(const char* configFile, helics_error* err);
helics_federate helicsFederateClone(helics_federate fed, helics_error* err);
helics_federate_info helicsCreateFederateInfo(void);
helics_federate_info helicsFederateInfoClone(helics_federate_info fi, helics_error* err);
void helicsFederateInfoLoadFromArgs(helics_federate_info fi, int argc, const char* const* argv, helics_error* err);
void helicsFederateInfoFree(helics_federate_info fi);
helics_bool helicsFederateIsValid(helics_federate fed);
void helicsFederateInfoSetCoreName(helics_federate_info fi, const char* corename, helics_error* err);
void helicsFederateInfoSetCoreInitString(helics_federate_info fi, const char* coreInit, helics_error* err);
void helicsFederateInfoSetBrokerInitString(helics_federate_info fi, const char* brokerInit, helics_error* err);
void helicsFederateInfoSetCoreType(helics_federate_info fi, int coretype, helics_error* err);
void helicsFederateInfoSetCoreTypeFromString(helics_federate_info fi, const char* coretype, helics_error* err);
void helicsFederateInfoSetBroker(helics_federate_info fi, const char* broker, helics_error* err);
void helicsFederateInfoSetBrokerKey(helics_federate_info fi, const char* brokerkey, helics_error* err);
void helicsFederateInfoSetBrokerPort(helics_federate_info fi, int brokerPort, helics_error* err);
void helicsFederateInfoSetLocalPort(helics_federate_info fi, const char* localPort, helics_error* err);
int helicsGetPropertyIndex(const char* val);
int helicsGetFlagIndex(const char* val);
int helicsGetOptionIndex(const char* val);
int helicsGetOptionValue(const char* val);
void helicsFederateInfoSetFlagOption(helics_federate_info fi, int flag, helics_bool value, helics_error* err);
void helicsFederateInfoSetSeparator(helics_federate_info fi, char separator, helics_error* err);
void helicsFederateInfoSetTimeProperty(helics_federate_info fi, int timeProperty, helics_time propertyValue, helics_error* err);
void helicsFederateInfoSetIntegerProperty(helics_federate_info fi, int intProperty, int propertyValue, helics_error* err);
void helicsFederateRegisterInterfaces(helics_federate fed, const char* file, helics_error* err);
void helicsFederateGlobalError(helics_federate fed, int error_code, const char* error_string);
void helicsFederateLocalError(helics_federate fed, int error_code, const char* error_string);
void helicsFederateFinalize(helics_federate fed, helics_error* err);
void helicsFederateFinalizeAsync(helics_federate fed, helics_error* err);
void helicsFederateFinalizeComplete(helics_federate fed, helics_error* err);
void helicsFederateFree(helics_federate fed);
void helicsCloseLibrary(void);

void helicsFederateEnterInitializingMode(helics_federate fed, helics_error* err);
void helicsFederateEnterInitializingModeAsync(helics_federate fed, helics_error* err);
helics_bool helicsFederateIsAsyncOperationCompleted(helics_federate fed, helics_error* err);
void helicsFederateEnterInitializingModeComplete(helics_federate fed, helics_error* err);
void helicsFederateEnterExecutingMode(helics_federate fed, helics_error* err);
void helicsFederateEnterExecutingModeAsync(helics_federate fed, helics_error* err);
void helicsFederateEnterExecutingModeComplete(helics_federate fed, helics_error* err);
helics_iteration_result helicsFederateEnterExecutingModeIterative(helics_federate fed, helics_iteration_request iterate, helics_error* err);
void helicsFederateEnterExecutingModeIterativeAsync(helics_federate fed, helics_iteration_request iterate, helics_error* err);
helics_iteration_result helicsFederateEnterExecutingModeIterativeComplete(helics_federate fed, helics_error* err);
helics_federate_state helicsFederateGetState(helics_federate fed, helics_error* err);
helics_core helicsFederateGetCore(helics_federate fed, helics_error* err);
helics_time helicsFederateRequestTime(helics_federate fed, helics_time requestTime, helics_error* err);
helics_time helicsFederateRequestTimeAdvance(helics_federate fed, helics_time timeDelta, helics_error* err);
helics_time helicsFederateRequestNextStep(helics_federate fed, helics_error* err);
helics_time helicsFederateRequestTimeIterative(helics_federate fed,
                                               helics_time requestTime,
                                               helics_iteration_request iterate,
                                               helics_iteration_result* outIteration,
                                               helics_error* err);
void helicsFederateRequestTimeAsync(helics_federate fed, helics_time requestTime, helics_error* err);
helics_time helicsFederateRequestTimeComplete(helics_federate fed, helics_error* err);
void helicsFederateRequestTimeIterativeAsync(helics_federate fed,
                                             helics_time requestTime,
                                             helics_iteration_request iterate,
                                             helics_error* err);
helics_time helicsFederateRequestTimeIterativeComplete(helics_federate fed, helics_iteration_result* outIterate, helics_error* err);
const char* helicsFederateGetName(helics_federate fed);
void helicsFederateSetTimeProperty(helics_federate fed, int timeProperty, helics_time time, helics_error* err);
void helicsFederateSetFlagOption(helics_federate fed, int flag, helics_bool flagValue, helics_error* err);
void helicsFederateSetSeparator(helics_federate fed, char separator, helics_error* err);
void helicsFederateSetIntegerProperty(helics_federate fed, int intProperty, int propertyVal, helics_error* err);
helics_time helicsFederateGetTimeProperty(helics_federate fed, int timeProperty, helics_error* err);
helics_bool helicsFederateGetFlagOption(helics_federate fed, int flag, helics_error* err);
int helicsFederateGetIntegerProperty(helics_federate fed, int intProperty, helics_error* err);
helics_time helicsFederateGetCurrentTime(helics_federate fed, helics_error* err);
void helicsFederateSetGlobal(helics_federate fed, const char* valueName, const char* value, helics_error* err);
void helicsFederateAddDependency(helics_federate fed, const char* fedName, helics_error* err);
void helicsFederateSetLogFile(helics_federate fed, const char* logFile, helics_error* err);
void helicsFederateLogErrorMessage(helics_federate fed, const char* logmessage, helics_error* err);
void helicsFederateLogWarningMessage(helics_federate fed, const char* logmessage, helics_error* err);
void helicsFederateLogInfoMessage(helics_federate fed, const char* logmessage, helics_error* err);
void helicsFederateLogDebugMessage(helics_federate fed, const char* logmessage, helics_error* err);
void helicsFederateLogLevelMessage(helics_federate fed, int loglevel, const char* logmessage, helics_error* err);
void helicsFederateSendCommand(helics_federate fed, const char* target, const char* command, helics_error* err);
const char* helicsFederateGetCommand(helics_federate fed, helics_error* err);
const char* helicsFederateGetCommandSource(helics_federate fed, helics_error* err);
const char* helicsFederateWaitCommand(helics_federate fed, helics_error* err);
void helicsCoreSetGlobal(helics_core core, const char* valueName, const char* value, helics_error* err);
void helicsBrokerSetGlobal(helics_broker broker, const char* valueName, const char* value, helics_error* err);
void helicsCoreSendCommand(helics_core core, const char* target, const char* command, helics_error* err);
void helicsBrokerSendCommand(helics_broker broker, const char* target, const char* command, helics_error* err);
void helicsCoreSetLogFile(helics_core core, const char* logFileName, helics_error* err);
void helicsBrokerSetLogFile(helics_broker broker, const char* logFileName, helics_error* err);
void helicsBrokerSetTimeBarrier(helics_broker broker, helics_time barrierTime, helics_error* err);
void helicsBrokerClearTimeBarrier(helics_broker broker);
helics_query helicsCreateQuery(const char* target, const char* query);
const char* helicsQueryExecute(helics_query query, helics_federate fed, helics_error* err);
const char* helicsQueryCoreExecute(helics_query query, helics_core core, helics_error* err);
const char* helicsQueryBrokerExecute(helics_query query, helics_broker broker, helics_error* err);
void helicsQueryExecuteAsync(helics_query query, helics_federate fed, helics_error* err);
const char* helicsQueryExecuteComplete(helics_query query, helics_error* err);
helics_bool helicsQueryIsCompleted(helics_query query);
void helicsQuerySetTarget(helics_query query, const char* target, helics_error* err);
void helicsQuerySetQueryString(helics_query query, const char* queryString, helics_error* err);
void helicsQueryFree(helics_query query);
void helicsCleanupLibrary(void);

helics_input helicsFederateRegisterSubscription(helics_federate fed, const char* key, const char* units, helics_error* err);
helics_publication
    helicsFederateRegisterPublication(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err);
helics_publication
    helicsFederateRegisterTypePublication(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err);
helics_publication helicsFederateRegisterGlobalPublication(helics_federate fed,
                                                           const char* key,
                                                           helics_data_type type,
                                                           const char* units,
                                                           helics_error* err);
helics_publication helicsFederateRegisterGlobalTypePublication(helics_federate fed,
                                                               const char* key,
                                                               const char* type,
                                                               const char* units,
                                                               helics_error* err);
helics_input helicsFederateRegisterInput(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err);
helics_input helicsFederateRegisterTypeInput(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err);
helics_publication
    helicsFederateRegisterGlobalInput(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err);
helics_publication
    helicsFederateRegisterGlobalTypeInput(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err);
helics_publication helicsFederateGetPublication(helics_federate fed, const char* key, helics_error* err);
helics_publication helicsFederateGetPublicationByIndex(helics_federate fed, int index, helics_error* err);
helics_input helicsFederateGetInput(helics_federate fed, const char* key, helics_error* err);
helics_input helicsFederateGetInputByIndex(helics_federate fed, int index, helics_error* err);
helics_input helicsFederateGetSubscription(helics_federate fed, const char* key, helics_error* err);
void helicsFederateClearUpdates(helics_federate fed);
void helicsFederateRegisterFromPublicationJSON(helics_federate fed, const char* json, helics_error* err);
void helicsFederatePublishJSON(helics_federate fed, const char* json, helics_error* err);

helics_bool helicsPublicationIsValid(helics_publication pub);
void helicsPublicationPublishBytes(helics_publication pub, const void* data, int inputDataLength, helics_error* err);
void helicsPublicationPublishString(helics_publication pub, const char* str, helics_error* err);
void helicsPublicationPublishInteger(helics_publication pub, int64_t val, helics_error* err);
void helicsPublicationPublishBoolean(helics_publication pub, helics_bool val, helics_error* err);
void helicsPublicationPublishDouble(helics_publication pub, double val, helics_error* err);
void helicsPublicationPublishTime(helics_publication pub, helics_time val, helics_error* err);
void helicsPublicationPublishChar(helics_publication pub, char val, helics_error* err);
void helicsPublicationPublishComplex(helics_publication pub, double real, double imag, helics_error* err);
void helicsPublicationPublishVector(helics_publication pub, const double* vectorInput, int vectorLength, helics_error* err);
void helicsPublicationPublishNamedPoint(helics_publication pub, const char* str, double val, helics_error* err);
void helicsPublicationAddTarget(helics_publication pub, const char* target, helics_error* err);
helics_bool helicsInputIsValid(helics_input ipt);
void helicsInputAddTarget(helics_input ipt, const char* target, helics_error* err);

int helicsInputGetByteCount(helics_input ipt);
void helicsInputGetBytes(helics_input ipt, void* data, int maxDataLength, int* actualSize, helics_error* err);
int helicsInputGetStringSize(helics_input ipt);
void helicsInputGetString(helics_input ipt, char* outputString, int maxStringLength, int* actualLength, helics_error* err);
int64_t helicsInputGetInteger(helics_input ipt, helics_error* err);
helics_bool helicsInputGetBoolean(helics_input ipt, helics_error* err);
double helicsInputGetDouble(helics_input ipt, helics_error* err);
helics_time helicsInputGetTime(helics_input ipt, helics_error* err);
char helicsInputGetChar(helics_input ipt, helics_error* err);
helics_complex helicsInputGetComplexObject(helics_input ipt, helics_error* err);
void helicsInputGetComplex(helics_input ipt, double* real, double* imag, helics_error* err);
int helicsInputGetVectorSize(helics_input ipt);
void helicsInputGetVector(helics_input ipt, double data[], int maxLength, int* actualSize, helics_error* err);
void helicsInputGetNamedPoint(helics_input ipt, char* outputString, int maxStringLength, int* actualLength, double* val, helics_error* err);

void helicsInputSetDefaultBytes(helics_input ipt, const void* data, int inputDataLength, helics_error* err);
void helicsInputSetDefaultString(helics_input ipt, const char* str, helics_error* err);
void helicsInputSetDefaultInteger(helics_input ipt, int64_t val, helics_error* err);
void helicsInputSetDefaultBoolean(helics_input ipt, helics_bool val, helics_error* err);
void helicsInputSetDefaultTime(helics_input ipt, helics_time val, helics_error* err);
void helicsInputSetDefaultChar(helics_input ipt, char val, helics_error* err);
void helicsInputSetDefaultDouble(helics_input ipt, double val, helics_error* err);
void helicsInputSetDefaultComplex(helics_input ipt, double real, double imag, helics_error* err);
void helicsInputSetDefaultVector(helics_input ipt, const double* vectorInput, int vectorLength, helics_error* err);
void helicsInputSetDefaultNamedPoint(helics_input ipt, const char* str, double val, helics_error* err);

const char* helicsInputGetType(helics_input ipt);
const char* helicsInputGetPublicationType(helics_input ipt);
const char* helicsPublicationGetType(helics_publication pub);
const char* helicsInputGetKey(helics_input ipt);
const char* helicsSubscriptionGetKey(helics_input ipt);
const char* helicsPublicationGetKey(helics_publication pub);
const char* helicsInputGetUnits(helics_input ipt);
const char* helicsInputGetInjectionUnits(helics_input ipt);
const char* helicsInputGetExtractionUnits(helics_input ipt);
const char* helicsPublicationGetUnits(helics_publication pub);
const char* helicsInputGetInfo(helics_input inp);
void helicsInputSetInfo(helics_input inp, const char* info, helics_error* err);
const char* helicsPublicationGetInfo(helics_publication pub);
void helicsPublicationSetInfo(helics_publication pub, const char* info, helics_error* err);
int helicsInputGetOption(helics_input inp, int option);
void helicsInputSetOption(helics_input inp, int option, int value, helics_error* err);
int helicsPublicationGetOption(helics_publication pub, int option);
void helicsPublicationSetOption(helics_publication pub, int option, int val, helics_error* err);
void helicsPublicationSetMinimumChange(helics_publication pub, double tolerance, helics_error* err);
void helicsInputSetMinimumChange(helics_input inp, double tolerance, helics_error* err);

helics_bool helicsInputIsUpdated(helics_input ipt);
helics_time helicsInputLastUpdateTime(helics_input ipt);
void helicsInputClearUpdate(helics_input ipt);
int helicsFederateGetPublicationCount(helics_federate fed);
int helicsFederateGetInputCount(helics_federate fed);

helics_endpoint helicsFederateRegisterEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err);
helics_endpoint helicsFederateRegisterGlobalEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err);
helics_endpoint helicsFederateRegisterTargetedEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err);
helics_endpoint helicsFederateRegisterGlobalTargetedEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err);
helics_endpoint helicsFederateGetEndpoint(helics_federate fed, const char* name, helics_error* err);
helics_endpoint helicsFederateGetEndpointByIndex(helics_federate fed, int index, helics_error* err);
helics_bool helicsEndpointIsValid(helics_endpoint endpoint);
void helicsEndpointSetDefaultDestination(helics_endpoint endpoint, const char* dst, helics_error* err);
const char* helicsEndpointGetDefaultDestination(helics_endpoint endpoint);
void helicsEndpointSendBytes(helics_endpoint endpoint, const void* data, int inputDataLength, helics_error* err);
void helicsEndpointSendBytesTo(helics_endpoint endpoint, const void* data, int inputDataLength, const char* dst, helics_error* err);
void helicsEndpointSendBytesToAt(helics_endpoint endpoint,
                                 const void* data,
                                 int inputDataLength,
                                 const char* dst,
                                 helics_time time,
                                 helics_error* err);
void helicsEndpointSendBytesAt(helics_endpoint endpoint, const void* data, int inputDataLength, helics_time time, helics_error* err);
void helicsEndpointSendMessage(helics_endpoint endpoint, helics_message message, helics_error* err);
void helicsEndpointSendMessageZeroCopy(helics_endpoint endpoint, helics_message message, helics_error* err);
void helicsEndpointSubscribe(helics_endpoint endpoint, const char* key, helics_error* err);
helics_bool helicsFederateHasMessage(helics_federate fed);
helics_bool helicsEndpointHasMessage(helics_endpoint endpoint);
int helicsFederatePendingMessagesCount(helics_federate fed);
int helicsEndpointPendingMessagesCount(helics_endpoint endpoint);
helics_message helicsEndpointGetMessage(helics_endpoint endpoint);
helics_message helicsEndpointCreateMessage(helics_endpoint endpoint, helics_error* err);
helics_message helicsFederateGetMessage(helics_federate fed);
helics_message helicsFederateCreateMessage(helics_federate fed, helics_error* err);
void helicsFederateClearMessages(helics_federate fed);
const char* helicsEndpointGetType(helics_endpoint endpoint);
const char* helicsEndpointGetName(helics_endpoint endpoint);
int helicsFederateGetEndpointCount(helics_federate fed);
const char* helicsEndpointGetInfo(helics_endpoint end);
void helicsEndpointSetInfo(helics_endpoint endpoint, const char* info, helics_error* err);
void helicsEndpointSetOption(helics_endpoint endpoint, int option, int value, helics_error* err);
int helicsEndpointGetOption(helics_endpoint endpoint, int option);
void helicsEndpointAddSourceTarget(helics_endpoint endpoint, const char* targetEndpoint, helics_error* err);
void helicsEndpointAddDestinationTarget(helics_endpoint endpoint, const char* targetEndpoint, helics_error* err);
void helicsEndpointRemoveTarget(helics_endpoint endpoint, const char* targetEndpoint, helics_error* err);
void helicsEndpointAddSourceFilter(helics_endpoint endpoint, const char* filterName, helics_error* err);
void helicsEndpointAddDestinationFilter(helics_endpoint endpoint, const char* filterName, helics_error* err);

const char* helicsMessageGetSource(helics_message message);
const char* helicsMessageGetDestination(helics_message message);
const char* helicsMessageGetOriginalSource(helics_message message);
const char* helicsMessageGetOriginalDestination(helics_message message);
helics_time helicsMessageGetTime(helics_message message);
const char* helicsMessageGetString(helics_message message);
int helicsMessageGetMessageID(helics_message message);
helics_bool helicsMessageGetFlagOption(helics_message message, int flag);
int helicsMessageGetByteCount(helics_message message);
void helicsMessageGetBytes(helics_message message, void* data, int maxMessageLength, int* actualSize, helics_error* err);
void* helicsMessageGetBytesPointer(helics_message message);
helics_bool helicsMessageIsValid(helics_message message);
void helicsMessageSetSource(helics_message message, const char* src, helics_error* err);
void helicsMessageSetDestination(helics_message message, const char* dst, helics_error* err);
void helicsMessageSetOriginalSource(helics_message message, const char* src, helics_error* err);
void helicsMessageSetOriginalDestination(helics_message message, const char* dst, helics_error* err);
void helicsMessageSetTime(helics_message message, helics_time time, helics_error* err);
void helicsMessageResize(helics_message message, int newSize, helics_error* err);
void helicsMessageReserve(helics_message message, int reserveSize, helics_error* err);
void helicsMessageSetMessageID(helics_message message, int32_t messageID, helics_error* err);
void helicsMessageClearFlags(helics_message message);
void helicsMessageSetFlagOption(helics_message message, int flag, helics_bool flagValue, helics_error* err);
void helicsMessageSetString(helics_message message, const char* str, helics_error* err);
void helicsMessageSetData(helics_message message, const void* data, int inputDataLength, helics_error* err);
void helicsMessageAppendData(helics_message message, const void* data, int inputDataLength, helics_error* err);
void helicsMessageCopy(helics_message src_message, helics_message dst_message, helics_error* err);
helics_message helicsMessageClone(helics_message message, helics_error* err);
void helicsMessageFree(helics_message message);
void helicsMessageClear(helics_message message, helics_error* err);

helics_filter helicsFederateRegisterFilter(helics_federate fed, helics_filter_type type, const char* name, helics_error* err);
helics_filter helicsFederateRegisterGlobalFilter(helics_federate fed, helics_filter_type type, const char* name, helics_error* err);
helics_filter helicsFederateRegisterCloningFilter(helics_federate fed, const char* name, helics_error* err);
helics_filter helicsFederateRegisterGlobalCloningFilter(helics_federate fed, const char* name, helics_error* err);
helics_filter helicsCoreRegisterFilter(helics_core core, helics_filter_type type, const char* name, helics_error* err);
helics_filter helicsCoreRegisterCloningFilter(helics_core core, const char* name, helics_error* err);
int helicsFederateGetFilterCount(helics_federate fed);
helics_filter helicsFederateGetFilter(helics_federate fed, const char* name, helics_error* err);
helics_filter helicsFederateGetFilterByIndex(helics_federate fed, int index, helics_error* err);
helics_bool helicsFilterIsValid(helics_filter filt);
const char* helicsFilterGetName(helics_filter filt);
void helicsFilterSet(helics_filter filt, const char* prop, double val, helics_error* err);
void helicsFilterSetString(helics_filter filt, const char* prop, const char* val, helics_error* err);
void helicsFilterAddDestinationTarget(helics_filter filt, const char* dst, helics_error* err);
void helicsFilterAddSourceTarget(helics_filter filt, const char* source, helics_error* err);

void helicsFilterAddDeliveryEndpoint(helics_filter filt, const char* deliveryEndpoint, helics_error* err);
void helicsFilterRemoveTarget(helics_filter filt, const char* target, helics_error* err);
void helicsFilterRemoveDeliveryEndpoint(helics_filter filt, const char* deliveryEndpoint, helics_error* err);
const char* helicsFilterGetInfo(helics_filter filt);
void helicsFilterSetInfo(helics_filter filt, const char* info, helics_error* err);
void helicsFilterSetOption(helics_filter filt, int option, int value, helics_error* err);
int helicsFilterGetOption(helics_filter filt, int option);

void helicsBrokerSetLoggingCallback(helics_broker broker,
                                    void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                    void* userdata,
                                    helics_error* err);
void helicsCoreSetLoggingCallback(helics_core core,
                                  void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                  void* userdata,
                                  helics_error* err);
void helicsFederateSetLoggingCallback(helics_federate fed,
                                      void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                      void* userdata,
                                      helics_error* err);
void helicsFilterSetCustomCallback(helics_filter filter,
                                   void (*filtCall)(helics_message message, void* userData),
                                   void* userdata,
                                   helics_error* err);
void helicsFederateSetQueryCallback(helics_federate fed,
                                    void (*queryAnswer)(const char* query, int querySize, helics_query_buffer buffer, void* userdata),
                                    void* userdata,
                                    helics_error* err);
void helicsQueryBufferFill(helics_query_buffer buffer, const char* str, int strSize, helics_error* err);
