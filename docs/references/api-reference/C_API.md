# C API Reference

## Table of Contents

1. [Enums](#enums)
1. [General function](#general)
1. [Creation functions](#creation)
1. [Broker methods](#broker)
1. [Core methods](#core)
1. [FederateInfo methods](#federateinfo)
1. [Federate methods](#federate)
1. [ValueFederate](#valuefederate)
1. [Publication methods](#publication)
1. [Input methods](#input)
1. [MessageFederate methods](#messagefederate)
1. [Endpoint methods](#endpoint)
1. [Message object methods](#message)
1. [FilterFederate methods](#filterfederate)
1. [Filter methods](#filter)
1. [Query methods](#query)

## Enums

```{eval-rst}

.. doxygenenumvalue:: HELICS_ITERATION_REQUEST_NO_ITERATION
    :project: helics

.. doxygenenumvalue:: HELICS_ITERATION_REQUEST_FORCE_ITERATION
    :project: helics

.. doxygenenumvalue:: HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED
    :project: helics

.. doxygenenumvalue:: HELICS_ITERATION_RESULT_NEXT_STEP
    :project: helics

.. doxygenenumvalue:: HELICS_ITERATION_RESULT_ERROR
    :project: helics

.. doxygenenumvalue:: HELICS_ITERATION_RESULT_HALTED
    :project: helics

.. doxygenenumvalue:: HELICS_ITERATION_RESULT_ITERATING
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_STARTUP
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_INITIALIZATION
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_EXECUTION
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_FINALIZE
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_ERROR
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_PENDING_INIT
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_PENDING_EXEC
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_PENDING_TIME
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_PENDING_ITERATIVE_TIME
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_PENDING_FINALIZE
    :project: helics

.. doxygenenumvalue:: HELICS_STATE_FINISHED
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_DEFAULT
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_ZMQ
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_MPI
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_TEST
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_INTERPROCESS
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_IPC
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_TCP
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_UDP
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_ZMQ_SS
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_NNG
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_TCP_SS
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_HTTP
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_WEBSOCKET
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_INPROC
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_NULL
    :project: helics

.. doxygenenumvalue:: HELICS_CORE_TYPE_EMPTY
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_UNKNOWN
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_STRING
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_DOUBLE
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_INT
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_COMPLEX
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_VECTOR
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_COMPLEX_VECTOR
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_NAMED_POINT
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_BOOLEAN
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_TIME
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_RAW
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_JSON
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_MULTI
    :project: helics

.. doxygenenumvalue:: HELICS_DATA_TYPE_ANY
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_OBSERVER
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_UNINTERRUPTIBLE
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_INTERRUPTIBLE
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_SOURCE_ONLY
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_ONLY_UPDATE_ON_CHANGE
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_RESTRICTIVE_TIME_POLICY
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_ROLLBACK
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_FORWARD_COMPUTE
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_REALTIME
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_SINGLE_THREAD_FEDERATE
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_STRICT_CONFIG_CHECKING
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_USE_JSON_SERIALIZATION
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_EVENT_TRIGGERED
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_LOCAL_PROFILING_CAPTURE
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_DELAY_INIT_ENTRY
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_ENABLE_INIT_ENTRY
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_IGNORE
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_SLOW_RESPONDING
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_DEBUGGING
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_TERMINATE_ON_ERROR
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_FORCE_LOGGING_FLUSH
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_DUMPLOG
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_PROFILING
    :project: helics

.. doxygenenumvalue:: HELICS_FLAG_PROFILING_MARKER
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_DUMPLOG
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_NO_PRINT
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_ERROR
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_PROFILING
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_WARNING
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_SUMMARY
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_CONNECTIONS
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_INTERFACES
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_TIMING
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_DATA
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_DEBUG
    :project: helics

.. doxygenenumvalue:: HELICS_LOG_LEVEL_TRACE
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_FATAL
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_EXTERNAL_TYPE
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_OTHER
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_USER_ABORT
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_INSUFFICIENT_SPACE
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_EXECUTION_FAILURE
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_INVALID_FUNCTION_CALL
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_INVALID_STATE_TRANSITION
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_SYSTEM_FAILURE
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_DISCARD
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_INVALID_ARGUMENT
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_INVALID_OBJECT
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_CONNECTION_FAILURE
    :project: helics

.. doxygenenumvalue:: HELICS_ERROR_REGISTRATION_FAILURE
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_TIME_DELTA
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_TIME_PERIOD
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_TIME_OFFSET
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_TIME_RT_LAG
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_TIME_RT_LEAD
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_TIME_RT_TOLERANCE
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_TIME_INPUT_DELAY
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_TIME_OUTPUT_DELAY
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_TIME_GRANT_TIMEOUT
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_INT_MAX_ITERATIONS
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_INT_LOG_LEVEL
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_INT_FILE_LOG_LEVEL
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL
    :project: helics

.. doxygenenumvalue:: HELICS_PROPERTY_INT_LOG_BUFFER
    :project: helics

.. doxygenenumvalue:: HELICS_MULTI_INPUT_NO_OP
    :project: helics

.. doxygenenumvalue:: HELICS_MULTI_INPUT_VECTORIZE_OPERATION
    :project: helics

.. doxygenenumvalue:: HELICS_MULTI_INPUT_AND_OPERATION
    :project: helics

.. doxygenenumvalue:: HELICS_MULTI_INPUT_OR_OPERATION
    :project: helics

.. doxygenenumvalue:: HELICS_MULTI_INPUT_SUM_OPERATION
    :project: helics

.. doxygenenumvalue:: HELICS_MULTI_INPUT_DIFF_OPERATION
    :project: helics

.. doxygenenumvalue:: HELICS_MULTI_INPUT_MAX_OPERATION
    :project: helics

.. doxygenenumvalue:: HELICS_MULTI_INPUT_MIN_OPERATION
    :project: helics

.. doxygenenumvalue:: HELICS_MULTI_INPUT_AVERAGE_OPERATION
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_CONNECTION_REQUIRED
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_BUFFER_DATA
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST
    :project: helics

.. doxygenenumvalue:: HELICS_HANDLE_OPTION_CONNECTIONS
    :project: helics

.. doxygenenumvalue:: HELICS_FILTER_TYPE_CUSTOM
    :project: helics

.. doxygenenumvalue:: HELICS_FILTER_TYPE_DELAY
    :project: helics

.. doxygenenumvalue:: HELICS_FILTER_TYPE_RANDOM_DELAY
    :project: helics

.. doxygenenumvalue:: HELICS_FILTER_TYPE_RANDOM_DROP
    :project: helics

.. doxygenenumvalue:: HELICS_FILTER_TYPE_REROUTE
    :project: helics

.. doxygenenumvalue:: HELICS_FILTER_TYPE_CLONE
    :project: helics

.. doxygenenumvalue:: HELICS_FILTER_TYPE_FIREWALL
    :project: helics

.. doxygenenumvalue:: HELICS_SEQUENCING_MODE_FAST
    :project: helics

.. doxygenenumvalue:: HELICS_SEQUENCING_MODE_ORDERED
    :project: helics

.. doxygenenumvalue:: HELICS_SEQUENCING_MODE_DEFAULT
    :project: helics

```

### General

```{eval-rst}

.. doxygenfunction:: helicsGetVersion
    :project: helics

.. doxygenfunction:: helicsGetBuildFlags
    :project: helics

.. doxygenfunction:: helicsGetCompilerVersion
    :project: helics

.. doxygenfunction:: helicsGetSystemInfo
    :project: helics

.. doxygenfunction:: helicsErrorInitialize
    :project: helics

.. doxygenfunction:: helicsErrorClear
    :project: helics

.. doxygenfunction:: helicsLoadSignalHandler
    :project: helics

.. doxygenfunction:: helicsLoadThreadedSignalHandler
    :project: helics

.. doxygenfunction:: helicsClearSignalHandler
    :project: helics

.. doxygenfunction:: helicsLoadSignalHandlerCallback
    :project: helics

.. doxygenfunction:: helicsAbort
    :project: helics

.. doxygenfunction:: helicsIsCoreTypeAvailable
    :project: helics

.. doxygenfunction:: helicsGetFederateByName
    :project: helics

.. doxygenfunction:: helicsGetPropertyIndex
    :project: helics

.. doxygenfunction:: helicsGetFlagIndex
    :project: helics

.. doxygenfunction:: helicsGetOptionIndex
    :project: helics

.. doxygenfunction:: helicsGetOptionValue
    :project: helics

.. doxygenfunction:: helicsGetDataType
    :project: helics

.. doxygenfunction:: helicsCloseLibrary
    :project: helics

.. doxygenfunction:: helicsCleanupLibrary
    :project: helics

```

### Creation

```{eval-rst}

.. doxygenfunction:: helicsCreateCore
    :project: helics

.. doxygenfunction:: helicsCreateCoreFromArgs
    :project: helics

.. doxygenfunction:: helicsCreateBroker
    :project: helics

.. doxygenfunction:: helicsCreateBrokerFromArgs
    :project: helics

.. doxygenfunction:: helicsCreateValueFederate
    :project: helics

.. doxygenfunction:: helicsCreateValueFederateFromConfig
    :project: helics

.. doxygenfunction:: helicsCreateMessageFederate
    :project: helics

.. doxygenfunction:: helicsCreateMessageFederateFromConfig
    :project: helics

.. doxygenfunction:: helicsCreateCombinationFederate
    :project: helics

.. doxygenfunction:: helicsCreateCombinationFederateFromConfig
    :project: helics

.. doxygenfunction:: helicsCreateFederateInfo
    :project: helics

.. doxygenfunction:: helicsCreateQuery
    :project: helics

```

### Broker

```{eval-rst}

.. doxygenfunction:: helicsBrokerClone
    :project: helics

.. doxygenfunction:: helicsBrokerIsValid
    :project: helics

.. doxygenfunction:: helicsBrokerIsConnected
    :project: helics

.. doxygenfunction:: helicsBrokerDataLink
    :project: helics

.. doxygenfunction:: helicsBrokerAddSourceFilterToEndpoint
    :project: helics

.. doxygenfunction:: helicsBrokerAddDestinationFilterToEndpoint
    :project: helics

.. doxygenfunction:: helicsBrokerMakeConnections
    :project: helics

.. doxygenfunction:: helicsBrokerWaitForDisconnect
    :project: helics

.. doxygenfunction:: helicsBrokerGetIdentifier
    :project: helics

.. doxygenfunction:: helicsBrokerGetAddress
    :project: helics

.. doxygenfunction:: helicsBrokerDisconnect
    :project: helics

.. doxygenfunction:: helicsBrokerDestroy
    :project: helics

.. doxygenfunction:: helicsBrokerFree
    :project: helics

.. doxygenfunction:: helicsBrokerSetGlobal
    :project: helics

.. doxygenfunction:: helicsBrokerSendCommand
    :project: helics

.. doxygenfunction:: helicsBrokerSetLogFile
    :project: helics

.. doxygenfunction:: helicsBrokerSetTimeBarrier
    :project: helics

.. doxygenfunction:: helicsBrokerClearTimeBarrier
    :project: helics

.. doxygenfunction:: helicsBrokerGlobalError
    :project: helics

.. doxygenfunction:: helicsBrokerSetLoggingCallback
    :project: helics

```

### Core

```{eval-rst}

.. doxygenfunction:: helicsCoreClone
    :project: helics

.. doxygenfunction:: helicsCoreIsValid
    :project: helics

.. doxygenfunction:: helicsCoreWaitForDisconnect
    :project: helics

.. doxygenfunction:: helicsCoreIsConnected
    :project: helics

.. doxygenfunction:: helicsCoreDataLink
    :project: helics

.. doxygenfunction:: helicsCoreAddSourceFilterToEndpoint
    :project: helics

.. doxygenfunction:: helicsCoreAddDestinationFilterToEndpoint
    :project: helics

.. doxygenfunction:: helicsCoreMakeConnections
    :project: helics

.. doxygenfunction:: helicsCoreGetIdentifier
    :project: helics

.. doxygenfunction:: helicsCoreGetAddress
    :project: helics

.. doxygenfunction:: helicsCoreSetReadyToInit
    :project: helics

.. doxygenfunction:: helicsCoreConnect
    :project: helics

.. doxygenfunction:: helicsCoreDisconnect
    :project: helics

.. doxygenfunction:: helicsCoreDestroy
    :project: helics

.. doxygenfunction:: helicsCoreFree
    :project: helics

.. doxygenfunction:: helicsCoreSetGlobal
    :project: helics

.. doxygenfunction:: helicsCoreSendCommand
    :project: helics

.. doxygenfunction:: helicsCoreSetLogFile
    :project: helics

.. doxygenfunction:: helicsCoreGlobalError
    :project: helics

.. doxygenfunction:: helicsCoreSetLoggingCallback
    :project: helics

.. doxygenfunction:: helicsCoreRegisterFilter
    :project: helics

.. doxygenfunction:: helicsCoreRegisterCloningFilter
    :project: helics

```

### FederateInfo

```{eval-rst}

.. doxygenfunction:: helicsFederateInfoClone
    :project: helics

.. doxygenfunction:: helicsFederateInfoLoadFromArgs
    :project: helics

.. doxygenfunction:: helicsFederateInfoLoadFromString
    :project: helics

.. doxygenfunction:: helicsFederateInfoFree
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetCoreName
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetCoreInitString
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetBrokerInitString
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetCoreType
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetCoreTypeFromString
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetBroker
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetBrokerKey
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetBrokerPort
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetLocalPort
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetFlagOption
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetSeparator
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetTimeProperty
    :project: helics

.. doxygenfunction:: helicsFederateInfoSetIntegerProperty
    :project: helics

```

### Federate

```{eval-rst}

.. doxygenfunction:: helicsFederateDestroy
    :project: helics

.. doxygenfunction:: helicsFederateClone
    :project: helics

.. doxygenfunction:: helicsFederateIsValid
    :project: helics

.. doxygenfunction:: helicsFederateRegisterInterfaces
    :project: helics

.. doxygenfunction:: helicsFederateGlobalError
    :project: helics

.. doxygenfunction:: helicsFederateLocalError
    :project: helics

.. doxygenfunction:: helicsFederateFinalize
    :project: helics

.. doxygenfunction:: helicsFederateFinalizeAsync
    :project: helics

.. doxygenfunction:: helicsFederateFinalizeComplete
    :project: helics

.. doxygenfunction:: helicsFederateDisconnect
    :project: helics

.. doxygenfunction:: helicsFederateDisconnectAsync
    :project: helics

.. doxygenfunction:: helicsFederateDisconnectComplete
    :project: helics

.. doxygenfunction:: helicsFederateFree
    :project: helics

.. doxygenfunction:: helicsFederateEnterInitializingMode
    :project: helics

.. doxygenfunction:: helicsFederateEnterInitializingModeAsync
    :project: helics

.. doxygenfunction:: helicsFederateIsAsyncOperationCompleted
    :project: helics

.. doxygenfunction:: helicsFederateEnterInitializingModeComplete
    :project: helics

.. doxygenfunction:: helicsFederateEnterExecutingMode
    :project: helics

.. doxygenfunction:: helicsFederateEnterExecutingModeAsync
    :project: helics

.. doxygenfunction:: helicsFederateEnterExecutingModeComplete
    :project: helics

.. doxygenfunction:: helicsFederateEnterExecutingModeIterative
    :project: helics

.. doxygenfunction:: helicsFederateEnterExecutingModeIterativeAsync
    :project: helics

.. doxygenfunction:: helicsFederateEnterExecutingModeIterativeComplete
    :project: helics

.. doxygenfunction:: helicsFederateGetState
    :project: helics

.. doxygenfunction:: helicsFederateGetCore
    :project: helics

.. doxygenfunction:: helicsFederateRequestTime
    :project: helics

.. doxygenfunction:: helicsFederateRequestTimeAdvance
    :project: helics

.. doxygenfunction:: helicsFederateRequestNextStep
    :project: helics

.. doxygenfunction:: helicsFederateRequestTimeIterative
    :project: helics

.. doxygenfunction:: helicsFederateRequestTimeAsync
    :project: helics

.. doxygenfunction:: helicsFederateRequestTimeComplete
    :project: helics

.. doxygenfunction:: helicsFederateRequestTimeIterativeAsync
    :project: helics

.. doxygenfunction:: helicsFederateRequestTimeIterativeComplete
    :project: helics

.. doxygenfunction:: helicsFederateProcessCommunications
    :project: helics

.. doxygenfunction:: helicsFederateGetName
    :project: helics

.. doxygenfunction:: helicsFederateSetTimeProperty
    :project: helics

.. doxygenfunction:: helicsFederateSetFlagOption
    :project: helics

.. doxygenfunction:: helicsFederateSetSeparator
    :project: helics

.. doxygenfunction:: helicsFederateSetIntegerProperty
    :project: helics

.. doxygenfunction:: helicsFederateGetTimeProperty
    :project: helics

.. doxygenfunction:: helicsFederateGetFlagOption
    :project: helics

.. doxygenfunction:: helicsFederateGetIntegerProperty
    :project: helics

.. doxygenfunction:: helicsFederateGetCurrentTime
    :project: helics

.. doxygenfunction:: helicsFederateSetGlobal
    :project: helics

.. doxygenfunction:: helicsFederateSetTag
    :project: helics

.. doxygenfunction:: helicsFederateGetTag
    :project: helics

.. doxygenfunction:: helicsFederateAddDependency
    :project: helics

.. doxygenfunction:: helicsFederateSetLogFile
    :project: helics

.. doxygenfunction:: helicsFederateLogErrorMessage
    :project: helics

.. doxygenfunction:: helicsFederateLogWarningMessage
    :project: helics

.. doxygenfunction:: helicsFederateLogInfoMessage
    :project: helics

.. doxygenfunction:: helicsFederateLogDebugMessage
    :project: helics

.. doxygenfunction:: helicsFederateLogLevelMessage
    :project: helics

.. doxygenfunction:: helicsFederateSendCommand
    :project: helics

.. doxygenfunction:: helicsFederateGetCommand
    :project: helics

.. doxygenfunction:: helicsFederateGetCommandSource
    :project: helics

.. doxygenfunction:: helicsFederateWaitCommand
    :project: helics

.. doxygenfunction:: helicsFederateSetLoggingCallback
    :project: helics

.. doxygenfunction:: helicsFederateSetQueryCallback
    :project: helics

.. doxygenfunction:: helicsFederateSetTimeUpdateCallback
    :project: helics

```

### ValueFederate

```{eval-rst}

.. doxygenfunction:: helicsFederateRegisterSubscription
    :project: helics

.. doxygenfunction:: helicsFederateRegisterPublication
    :project: helics

.. doxygenfunction:: helicsFederateRegisterTypePublication
    :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalPublication
    :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalTypePublication
    :project: helics

.. doxygenfunction:: helicsFederateRegisterInput
    :project: helics

.. doxygenfunction:: helicsFederateRegisterTypeInput
    :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalInput
    :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalTypeInput
    :project: helics

.. doxygenfunction:: helicsFederateGetPublication
    :project: helics

.. doxygenfunction:: helicsFederateGetPublicationByIndex
    :project: helics

.. doxygenfunction:: helicsFederateGetInput
    :project: helics

.. doxygenfunction:: helicsFederateGetInputByIndex
    :project: helics

.. doxygenfunction:: helicsFederateGetSubscription
    :project: helics

.. doxygenfunction:: helicsFederateClearUpdates
    :project: helics

.. doxygenfunction:: helicsFederateRegisterFromPublicationJSON
    :project: helics

.. doxygenfunction:: helicsFederatePublishJSON
    :project: helics

.. doxygenfunction:: helicsFederateGetPublicationCount
    :project: helics

.. doxygenfunction:: helicsFederateGetInputCount
    :project: helics

```

### Publication

```{eval-rst}

.. doxygenfunction:: helicsPublicationIsValid
    :project: helics

.. doxygenfunction:: helicsPublicationPublishBytes
    :project: helics

.. doxygenfunction:: helicsPublicationPublishString
    :project: helics

.. doxygenfunction:: helicsPublicationPublishInteger
    :project: helics

.. doxygenfunction:: helicsPublicationPublishBoolean
    :project: helics

.. doxygenfunction:: helicsPublicationPublishDouble
    :project: helics

.. doxygenfunction:: helicsPublicationPublishTime
    :project: helics

.. doxygenfunction:: helicsPublicationPublishChar
    :project: helics

.. doxygenfunction:: helicsPublicationPublishComplex
    :project: helics

.. doxygenfunction:: helicsPublicationPublishVector
    :project: helics

.. doxygenfunction:: helicsPublicationPublishComplexVector
    :project: helics

.. doxygenfunction:: helicsPublicationPublishNamedPoint
    :project: helics

.. doxygenfunction:: helicsPublicationAddTarget
    :project: helics

.. doxygenfunction:: helicsPublicationGetType
    :project: helics

.. doxygenfunction:: helicsPublicationGetName
    :project: helics

.. doxygenfunction:: helicsPublicationGetUnits
    :project: helics

.. doxygenfunction:: helicsPublicationGetInfo
    :project: helics

.. doxygenfunction:: helicsPublicationSetInfo
    :project: helics

.. doxygenfunction:: helicsPublicationGetTag
    :project: helics

.. doxygenfunction:: helicsPublicationSetTag
    :project: helics

.. doxygenfunction:: helicsPublicationGetOption
    :project: helics

.. doxygenfunction:: helicsPublicationSetOption
    :project: helics

.. doxygenfunction:: helicsPublicationSetMinimumChange
    :project: helics

```

### Input

```{eval-rst}

.. doxygenfunction:: helicsInputIsValid
    :project: helics

.. doxygenfunction:: helicsInputAddTarget
    :project: helics

.. doxygenfunction:: helicsInputGetByteCount
    :project: helics

.. doxygenfunction:: helicsInputGetBytes
    :project: helics

.. doxygenfunction:: helicsInputGetStringSize
    :project: helics

.. doxygenfunction:: helicsInputGetString
    :project: helics

.. doxygenfunction:: helicsInputGetInteger
    :project: helics

.. doxygenfunction:: helicsInputGetBoolean
    :project: helics

.. doxygenfunction:: helicsInputGetDouble
    :project: helics

.. doxygenfunction:: helicsInputGetTime
    :project: helics

.. doxygenfunction:: helicsInputGetChar
    :project: helics

.. doxygenfunction:: helicsInputGetComplexObject
    :project: helics

.. doxygenfunction:: helicsInputGetComplex
    :project: helics

.. doxygenfunction:: helicsInputGetVectorSize
    :project: helics

.. doxygenfunction:: helicsInputGetVector
    :project: helics

.. doxygenfunction:: helicsInputGetComplexVector
    :project: helics

.. doxygenfunction:: helicsInputGetNamedPoint
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultBytes
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultString
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultInteger
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultBoolean
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultTime
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultChar
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultDouble
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultComplex
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultVector
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultComplexVector
    :project: helics

.. doxygenfunction:: helicsInputSetDefaultNamedPoint
    :project: helics

.. doxygenfunction:: helicsInputGetType
    :project: helics

.. doxygenfunction:: helicsInputGetPublicationType
    :project: helics

.. doxygenfunction:: helicsInputGetPublicationDataType
    :project: helics

.. doxygenfunction:: helicsInputGetName
    :project: helics

.. doxygenfunction:: helicsSubscriptionGetTarget
    :project: helics

.. doxygenfunction:: helicsInputGetUnits
    :project: helics

.. doxygenfunction:: helicsInputGetInjectionUnits
    :project: helics

.. doxygenfunction:: helicsInputGetExtractionUnits
    :project: helics

.. doxygenfunction:: helicsInputGetInfo
    :project: helics

.. doxygenfunction:: helicsInputSetInfo
    :project: helics

.. doxygenfunction:: helicsInputGetTag
    :project: helics

.. doxygenfunction:: helicsInputSetTag
    :project: helics

.. doxygenfunction:: helicsInputGetOption
    :project: helics

.. doxygenfunction:: helicsInputSetOption
    :project: helics

.. doxygenfunction:: helicsInputSetMinimumChange
    :project: helics

.. doxygenfunction:: helicsInputIsUpdated
    :project: helics

.. doxygenfunction:: helicsInputLastUpdateTime
    :project: helics

.. doxygenfunction:: helicsInputClearUpdate
    :project: helics

```

### MessageFederate

```{eval-rst}

.. doxygenfunction:: helicsFederateRegisterEndpoint
    :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalEndpoint
    :project: helics

.. doxygenfunction:: helicsFederateRegisterTargetedEndpoint
    :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalTargetedEndpoint
    :project: helics

.. doxygenfunction:: helicsFederateGetEndpoint
    :project: helics

.. doxygenfunction:: helicsFederateGetEndpointByIndex
    :project: helics

.. doxygenfunction:: helicsFederateHasMessage
    :project: helics

.. doxygenfunction:: helicsFederatePendingMessageCount
    :project: helics

.. doxygenfunction:: helicsFederateGetMessage
    :project: helics

.. doxygenfunction:: helicsFederateCreateMessage
    :project: helics

.. doxygenfunction:: helicsFederateClearMessages
    :project: helics

.. doxygenfunction:: helicsFederateGetEndpointCount
    :project: helics

```

### Endpoint

```{eval-rst}

.. doxygenfunction:: helicsEndpointIsValid
    :project: helics

.. doxygenfunction:: helicsEndpointSetDefaultDestination
    :project: helics

.. doxygenfunction:: helicsEndpointGetDefaultDestination
    :project: helics

.. doxygenfunction:: helicsEndpointSendBytes
    :project: helics

.. doxygenfunction:: helicsEndpointSendBytesTo
    :project: helics

.. doxygenfunction:: helicsEndpointSendBytesToAt
    :project: helics

.. doxygenfunction:: helicsEndpointSendBytesAt
    :project: helics

.. doxygenfunction:: helicsEndpointSendMessage
    :project: helics

.. doxygenfunction:: helicsEndpointSendMessageZeroCopy
    :project: helics

.. doxygenfunction:: helicsEndpointSubscribe
    :project: helics

.. doxygenfunction:: helicsEndpointHasMessage
    :project: helics

.. doxygenfunction:: helicsEndpointPendingMessageCount
    :project: helics

.. doxygenfunction:: helicsEndpointGetMessage
    :project: helics

.. doxygenfunction:: helicsEndpointCreateMessage
    :project: helics

.. doxygenfunction:: helicsEndpointGetType
    :project: helics

.. doxygenfunction:: helicsEndpointGetName
    :project: helics

.. doxygenfunction:: helicsEndpointGetInfo
    :project: helics

.. doxygenfunction:: helicsEndpointSetInfo
    :project: helics

.. doxygenfunction:: helicsEndpointGetTag
    :project: helics

.. doxygenfunction:: helicsEndpointSetTag
    :project: helics

.. doxygenfunction:: helicsEndpointSetOption
    :project: helics

.. doxygenfunction:: helicsEndpointGetOption
    :project: helics

.. doxygenfunction:: helicsEndpointAddSourceTarget
    :project: helics

.. doxygenfunction:: helicsEndpointAddDestinationTarget
    :project: helics

.. doxygenfunction:: helicsEndpointRemoveTarget
    :project: helics

.. doxygenfunction:: helicsEndpointAddSourceFilter
    :project: helics

.. doxygenfunction:: helicsEndpointAddDestinationFilter
    :project: helics

```

### Message

```{eval-rst}

.. doxygenfunction:: helicsMessageGetSource
    :project: helics

.. doxygenfunction:: helicsMessageGetDestination
    :project: helics

.. doxygenfunction:: helicsMessageGetOriginalSource
    :project: helics

.. doxygenfunction:: helicsMessageGetOriginalDestination
    :project: helics

.. doxygenfunction:: helicsMessageGetTime
    :project: helics

.. doxygenfunction:: helicsMessageGetString
    :project: helics

.. doxygenfunction:: helicsMessageGetMessageID
    :project: helics

.. doxygenfunction:: helicsMessageGetFlagOption
    :project: helics

.. doxygenfunction:: helicsMessageGetByteCount
    :project: helics

.. doxygenfunction:: helicsMessageGetBytes
    :project: helics

.. doxygenfunction:: helicsMessageIsValid
    :project: helics

.. doxygenfunction:: helicsMessageSetSource
    :project: helics

.. doxygenfunction:: helicsMessageSetDestination
    :project: helics

.. doxygenfunction:: helicsMessageSetOriginalSource
    :project: helics

.. doxygenfunction:: helicsMessageSetOriginalDestination
    :project: helics

.. doxygenfunction:: helicsMessageSetTime
    :project: helics

.. doxygenfunction:: helicsMessageResize
    :project: helics

.. doxygenfunction:: helicsMessageReserve
    :project: helics

.. doxygenfunction:: helicsMessageSetMessageID
    :project: helics

.. doxygenfunction:: helicsMessageClearFlags
    :project: helics

.. doxygenfunction:: helicsMessageSetFlagOption
    :project: helics

.. doxygenfunction:: helicsMessageSetString
    :project: helics

.. doxygenfunction:: helicsMessageSetData
    :project: helics

.. doxygenfunction:: helicsMessageAppendData
    :project: helics

.. doxygenfunction:: helicsMessageCopy
    :project: helics

.. doxygenfunction:: helicsMessageClone
    :project: helics

.. doxygenfunction:: helicsMessageFree
    :project: helics

.. doxygenfunction:: helicsMessageClear
    :project: helics

```

### FilterFederate

```{eval-rst}

.. doxygenfunction:: helicsFederateRegisterFilter
    :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalFilter
    :project: helics

.. doxygenfunction:: helicsFederateRegisterCloningFilter
    :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalCloningFilter
    :project: helics

.. doxygenfunction:: helicsFederateGetFilterCount
    :project: helics

.. doxygenfunction:: helicsFederateGetFilter
    :project: helics

.. doxygenfunction:: helicsFederateGetFilterByIndex
    :project: helics

```

### Filter

```{eval-rst}

.. doxygenfunction:: helicsFilterSetCustomCallback
    :project: helics

.. doxygenfunction:: helicsFilterIsValid
    :project: helics

.. doxygenfunction:: helicsFilterGetName
    :project: helics

.. doxygenfunction:: helicsFilterSet
    :project: helics

.. doxygenfunction:: helicsFilterSetString
    :project: helics

.. doxygenfunction:: helicsFilterAddDestinationTarget
    :project: helics

.. doxygenfunction:: helicsFilterAddSourceTarget
    :project: helics

.. doxygenfunction:: helicsFilterAddDeliveryEndpoint
    :project: helics

.. doxygenfunction:: helicsFilterRemoveTarget
    :project: helics

.. doxygenfunction:: helicsFilterRemoveDeliveryEndpoint
    :project: helics

.. doxygenfunction:: helicsFilterGetInfo
    :project: helics

.. doxygenfunction:: helicsFilterSetInfo
    :project: helics

.. doxygenfunction:: helicsFilterGetTag
    :project: helics

.. doxygenfunction:: helicsFilterSetTag
    :project: helics

.. doxygenfunction:: helicsFilterSetOption
    :project: helics

.. doxygenfunction:: helicsFilterGetOption
    :project: helics
```

### Query

```{eval-rst}

.. doxygenfunction:: helicsQueryExecute
    :project: helics

.. doxygenfunction:: helicsQueryCoreExecute
    :project: helics

.. doxygenfunction:: helicsQueryBrokerExecute
    :project: helics

.. doxygenfunction:: helicsQueryExecuteAsync
    :project: helics

.. doxygenfunction:: helicsQueryExecuteComplete
    :project: helics

.. doxygenfunction:: helicsQueryIsCompleted
    :project: helics

.. doxygenfunction:: helicsQuerySetTarget
    :project: helics

.. doxygenfunction:: helicsQuerySetQueryString
    :project: helics

.. doxygenfunction:: helicsQuerySetOrdering
    :project: helics

.. doxygenfunction:: helicsQueryFree
    :project: helics

.. doxygenfunction:: helicsQueryBufferFill
    :project: helics
```
