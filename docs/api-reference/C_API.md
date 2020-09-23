# C API Reference

## Enum

```eval_rst

.. doxygenenumvalue:: helics_core_type_default
    :project: helics


.. doxygenenumvalue:: helics_core_type_http
    :project: helics


.. doxygenenumvalue:: helics_core_type_inproc
    :project: helics


.. doxygenenumvalue:: helics_core_type_interprocess
    :project: helics


.. doxygenenumvalue:: helics_core_type_ipc
    :project: helics


.. doxygenenumvalue:: helics_core_type_mpi
    :project: helics


.. doxygenenumvalue:: helics_core_type_nng
    :project: helics


.. doxygenenumvalue:: helics_core_type_null
    :project: helics


.. doxygenenumvalue:: helics_core_type_tcp
    :project: helics


.. doxygenenumvalue:: helics_core_type_tcp_ss
    :project: helics


.. doxygenenumvalue:: helics_core_type_test
    :project: helics


.. doxygenenumvalue:: helics_core_type_udp
    :project: helics


.. doxygenenumvalue:: helics_core_type_websocket
    :project: helics


.. doxygenenumvalue:: helics_core_type_zmq
    :project: helics


.. doxygenenumvalue:: helics_core_type_zmq_test
    :project: helics


.. doxygenenumvalue:: helics_data_type_any
    :project: helics


.. doxygenenumvalue:: helics_data_type_boolean
    :project: helics


.. doxygenenumvalue:: helics_data_type_complex
    :project: helics


.. doxygenenumvalue:: helics_data_type_complex_vector
    :project: helics


.. doxygenenumvalue:: helics_data_type_double
    :project: helics


.. doxygenenumvalue:: helics_data_type_int
    :project: helics


.. doxygenenumvalue:: helics_data_type_named_point
    :project: helics


.. doxygenenumvalue:: helics_data_type_raw
    :project: helics


.. doxygenenumvalue:: helics_data_type_string
    :project: helics


.. doxygenenumvalue:: helics_data_type_time
    :project: helics


.. doxygenenumvalue:: helics_data_type_vector
    :project: helics


.. doxygenenumvalue:: helics_error_connection_failure
    :project: helics


.. doxygenenumvalue:: helics_error_discard
    :project: helics


.. doxygenenumvalue:: helics_error_execution_failure
    :project: helics


.. doxygenenumvalue:: helics_error_external_type
    :project: helics


.. doxygenenumvalue:: helics_error_fatal
    :project: helics


.. doxygenenumvalue:: helics_error_insufficient_space
    :project: helics


.. doxygenenumvalue:: helics_error_invalid_argument
    :project: helics


.. doxygenenumvalue:: helics_error_invalid_function_call
    :project: helics


.. doxygenenumvalue:: helics_error_invalid_object
    :project: helics


.. doxygenenumvalue:: helics_error_invalid_state_transition
    :project: helics


.. doxygenenumvalue:: helics_error_other
    :project: helics


.. doxygenenumvalue:: helics_error_registration_failure
    :project: helics


.. doxygenenumvalue:: helics_error_system_failure
    :project: helics


.. doxygenenumvalue:: helics_filter_type_clone
    :project: helics


.. doxygenenumvalue:: helics_filter_type_custom
    :project: helics


.. doxygenenumvalue:: helics_filter_type_delay
    :project: helics


.. doxygenenumvalue:: helics_filter_type_firewall
    :project: helics


.. doxygenenumvalue:: helics_filter_type_random_delay
    :project: helics


.. doxygenenumvalue:: helics_filter_type_random_drop
    :project: helics


.. doxygenenumvalue:: helics_filter_type_reroute
    :project: helics


.. doxygenenumvalue:: helics_flag_delay_init_entry
    :project: helics


.. doxygenenumvalue:: helics_flag_enable_init_entry
    :project: helics


.. doxygenenumvalue:: helics_flag_forward_compute
    :project: helics


.. doxygenenumvalue:: helics_flag_ignore_time_mismatch_warnings
    :project: helics


.. doxygenenumvalue:: helics_flag_interruptible
    :project: helics


.. doxygenenumvalue:: helics_flag_observer
    :project: helics


.. doxygenenumvalue:: helics_flag_only_transmit_on_change
    :project: helics


.. doxygenenumvalue:: helics_flag_only_update_on_change
    :project: helics


.. doxygenenumvalue:: helics_flag_realtime
    :project: helics


.. doxygenenumvalue:: helics_flag_restrictive_time_policy
    :project: helics


.. doxygenenumvalue:: helics_flag_rollback
    :project: helics


.. doxygenenumvalue:: helics_flag_single_thread_federate
    :project: helics


.. doxygenenumvalue:: helics_flag_slow_responding
    :project: helics


.. doxygenenumvalue:: helics_flag_source_only
    :project: helics


.. doxygenenumvalue:: helics_flag_terminate_on_error
    :project: helics


.. doxygenenumvalue:: helics_flag_uninterruptible
    :project: helics


.. doxygenenumvalue:: helics_flag_wait_for_current_time_update
    :project: helics


.. doxygenenumvalue:: helics_handle_option_buffer_data
    :project: helics


.. doxygenenumvalue:: helics_handle_option_connection_optional
    :project: helics


.. doxygenenumvalue:: helics_handle_option_connection_required
    :project: helics


.. doxygenenumvalue:: helics_handle_option_ignore_interrupts
    :project: helics


.. doxygenenumvalue:: helics_handle_option_ignore_unit_mismatch
    :project: helics


.. doxygenenumvalue:: helics_handle_option_multiple_connections_allowed
    :project: helics


.. doxygenenumvalue:: helics_handle_option_only_transmit_on_change
    :project: helics


.. doxygenenumvalue:: helics_handle_option_only_update_on_change
    :project: helics


.. doxygenenumvalue:: helics_handle_option_single_connection_only
    :project: helics


.. doxygenenumvalue:: helics_handle_option_strict_type_checking
    :project: helics


.. doxygenenumvalue:: helics_iteration_request_force_iteration
    :project: helics


.. doxygenenumvalue:: helics_iteration_request_iterate_if_needed
    :project: helics


.. doxygenenumvalue:: helics_iteration_request_no_iteration
    :project: helics


.. doxygenenumvalue:: helics_iteration_result_error
    :project: helics


.. doxygenenumvalue:: helics_iteration_result_halted
    :project: helics


.. doxygenenumvalue:: helics_iteration_result_iterating
    :project: helics


.. doxygenenumvalue:: helics_iteration_result_next_step
    :project: helics


.. doxygenenumvalue:: helics_log_level_connections
    :project: helics


.. doxygenenumvalue:: helics_log_level_data
    :project: helics


.. doxygenenumvalue:: helics_log_level_error
    :project: helics


.. doxygenenumvalue:: helics_log_level_interfaces
    :project: helics


.. doxygenenumvalue:: helics_log_level_no_print
    :project: helics


.. doxygenenumvalue:: helics_log_level_summary
    :project: helics


.. doxygenenumvalue:: helics_log_level_timing
    :project: helics


.. doxygenenumvalue:: helics_log_level_trace
    :project: helics


.. doxygenenumvalue:: helics_log_level_warning
    :project: helics


.. doxygenenumvalue:: helics_ok
    :project: helics


.. doxygenenumvalue:: helics_property_int_console_log_level
    :project: helics


.. doxygenenumvalue:: helics_property_int_file_log_level
    :project: helics


.. doxygenenumvalue:: helics_property_int_log_level
    :project: helics


.. doxygenenumvalue:: helics_property_int_max_iterations
    :project: helics


.. doxygenenumvalue:: helics_property_time_delta
    :project: helics


.. doxygenenumvalue:: helics_property_time_input_delay
    :project: helics


.. doxygenenumvalue:: helics_property_time_offset
    :project: helics


.. doxygenenumvalue:: helics_property_time_output_delay
    :project: helics


.. doxygenenumvalue:: helics_property_time_period
    :project: helics


.. doxygenenumvalue:: helics_property_time_rt_lag
    :project: helics


.. doxygenenumvalue:: helics_property_time_rt_lead
    :project: helics


.. doxygenenumvalue:: helics_property_time_rt_tolerance
    :project: helics


.. doxygenenumvalue:: helics_state_error
    :project: helics


.. doxygenenumvalue:: helics_state_execution
    :project: helics


.. doxygenenumvalue:: helics_state_finalize
    :project: helics


.. doxygenenumvalue:: helics_state_initialization
    :project: helics


.. doxygenenumvalue:: helics_state_pending_exec
    :project: helics


.. doxygenenumvalue:: helics_state_pending_finalize
    :project: helics


.. doxygenenumvalue:: helics_state_pending_init
    :project: helics


.. doxygenenumvalue:: helics_state_pending_iterative_time
    :project: helics


.. doxygenenumvalue:: helics_state_pending_time
    :project: helics


.. doxygenenumvalue:: helics_state_startup
    :project: helics


.. doxygenenumvalue:: helics_warning
    :project: helics

```

## Functions

1. `Broker`
1. `Core`
1. `Endpoint`
1. `FederateInfo`
1. `Federate`
1. `Filter`
1. `Input`
1. `Message`
1. `Publication`
1. `Query`

### Broker

```eval_rst

.. doxygenfunction:: helicsBrokerAddDestinationFilterToEndpoint
    :project: helics


.. doxygenfunction:: helicsBrokerAddSourceFilterToEndpoint
    :project: helics


.. doxygenfunction:: helicsBrokerClone
    :project: helics


.. doxygenfunction:: helicsBrokerDataLink
    :project: helics


.. doxygenfunction:: helicsBrokerDestroy
    :project: helics


.. doxygenfunction:: helicsBrokerDisconnect
    :project: helics


.. doxygenfunction:: helicsBrokerFree
    :project: helics


.. doxygenfunction:: helicsBrokerGetAddress
    :project: helics


.. doxygenfunction:: helicsBrokerGetIdentifier
    :project: helics


.. doxygenfunction:: helicsBrokerIsConnected
    :project: helics


.. doxygenfunction:: helicsBrokerIsValid
    :project: helics


.. doxygenfunction:: helicsBrokerMakeConnections
    :project: helics


.. doxygenfunction:: helicsBrokerSetGlobal
    :project: helics


.. doxygenfunction:: helicsBrokerSetLogFile
    :project: helics


.. doxygenfunction:: helicsBrokerWaitForDisconnect
    :project: helics


.. doxygenfunction:: helicsBrokerSetLoggingCallback
    :project: helics

```

### Core

```eval_rst

.. doxygenfunction:: helicsCoreAddDestinationFilterToEndpoint
    :project: helics


.. doxygenfunction:: helicsCoreAddSourceFilterToEndpoint
    :project: helics


.. doxygenfunction:: helicsCoreClone
    :project: helics


.. doxygenfunction:: helicsCoreConnect
    :project: helics


.. doxygenfunction:: helicsCoreDataLink
    :project: helics


.. doxygenfunction:: helicsCoreDestroy
    :project: helics


.. doxygenfunction:: helicsCoreDisconnect
    :project: helics


.. doxygenfunction:: helicsCoreFree
    :project: helics


.. doxygenfunction:: helicsCoreGetAddress
    :project: helics


.. doxygenfunction:: helicsCoreGetIdentifier
    :project: helics


.. doxygenfunction:: helicsCoreIsConnected
    :project: helics


.. doxygenfunction:: helicsCoreIsValid
    :project: helics


.. doxygenfunction:: helicsCoreMakeConnections
    :project: helics


.. doxygenfunction:: helicsCoreRegisterCloningFilter
    :project: helics


.. doxygenfunction:: helicsCoreRegisterFilter
    :project: helics


.. doxygenfunction:: helicsCoreSetGlobal
    :project: helics


.. doxygenfunction:: helicsCoreSetLogFile
    :project: helics


.. doxygenfunction:: helicsCoreSetReadyToInit
    :project: helics


.. doxygenfunction:: helicsCoreWaitForDisconnect
    :project: helics


.. doxygenfunction:: helicsCoreSetLoggingCallback
    :project: helics

```

### Endpoint

```eval_rst

.. doxygenfunction:: helicsEndpointClearMessages
    :project: helics


.. doxygenfunction:: helicsEndpointCreateMessageObject
    :project: helics


.. doxygenfunction:: helicsEndpointGetDefaultDestination
    :project: helics


.. doxygenfunction:: helicsEndpointGetInfo
    :project: helics


.. doxygenfunction:: helicsEndpointGetMessage
    :project: helics


.. doxygenfunction:: helicsEndpointGetMessageObject
    :project: helics


.. doxygenfunction:: helicsEndpointGetName
    :project: helics


.. doxygenfunction:: helicsEndpointGetOption
    :project: helics


.. doxygenfunction:: helicsEndpointGetType
    :project: helics


.. doxygenfunction:: helicsEndpointHasMessage
    :project: helics


.. doxygenfunction:: helicsEndpointIsValid
    :project: helics


.. doxygenfunction:: helicsEndpointPendingMessages
    :project: helics


.. doxygenfunction:: helicsEndpointSendEventRaw
    :project: helics


.. doxygenfunction:: helicsEndpointSendMessage
    :project: helics


.. doxygenfunction:: helicsEndpointSendMessageObject
    :project: helics


.. doxygenfunction:: helicsEndpointSendMessageObjectZeroCopy
    :project: helics


.. doxygenfunction:: helicsEndpointSendMessageRaw
    :project: helics


.. doxygenfunction:: helicsEndpointSetDefaultDestination
    :project: helics


.. doxygenfunction:: helicsEndpointSetInfo
    :project: helics


.. doxygenfunction:: helicsEndpointSetOption
    :project: helics


.. doxygenfunction:: helicsEndpointSubscribe
    :project: helics

```

### FederateInfo

```eval_rst

.. doxygenfunction:: helicsFederateInfoClone
    :project: helics


.. doxygenfunction:: helicsFederateInfoFree
    :project: helics


.. doxygenfunction:: helicsFederateInfoLoadFromArgs
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetBroker
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetBrokerInitString
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetBrokerKey
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetBrokerPort
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetCoreInitString
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetCoreName
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetCoreType
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetCoreTypeFromString
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetFlagOption
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetIntegerProperty
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetLocalPort
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetSeparator
    :project: helics


.. doxygenfunction:: helicsFederateInfoSetTimeProperty
    :project: helics

```

### Federate

```eval_rst

.. doxygenfunction:: helicsFederateAddDependency
    :project: helics


.. doxygenfunction:: helicsFederateClearMessages
    :project: helics


.. doxygenfunction:: helicsFederateClearUpdates
    :project: helics


.. doxygenfunction:: helicsFederateClone
    :project: helics


.. doxygenfunction:: helicsFederateCreateMessageObject
    :project: helics


.. doxygenfunction:: helicsFederateDestroy
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


.. doxygenfunction:: helicsFederateEnterInitializingMode
    :project: helics


.. doxygenfunction:: helicsFederateEnterInitializingModeAsync
    :project: helics


.. doxygenfunction:: helicsFederateEnterInitializingModeComplete
    :project: helics


.. doxygenfunction:: helicsFederateFinalize
    :project: helics


.. doxygenfunction:: helicsFederateFinalizeAsync
    :project: helics


.. doxygenfunction:: helicsFederateFinalizeComplete
    :project: helics


.. doxygenfunction:: helicsFederateFree
    :project: helics


.. doxygenfunction:: helicsFederateGetCoreObject
    :project: helics


.. doxygenfunction:: helicsFederateGetCurrentTime
    :project: helics


.. doxygenfunction:: helicsFederateGetEndpoint
    :project: helics


.. doxygenfunction:: helicsFederateGetEndpointByIndex
    :project: helics


.. doxygenfunction:: helicsFederateGetEndpointCount
    :project: helics


.. doxygenfunction:: helicsFederateGetFilter
    :project: helics


.. doxygenfunction:: helicsFederateGetFilterByIndex
    :project: helics


.. doxygenfunction:: helicsFederateGetFilterCount
    :project: helics


.. doxygenfunction:: helicsFederateGetFlagOption
    :project: helics


.. doxygenfunction:: helicsFederateGetInput
    :project: helics


.. doxygenfunction:: helicsFederateGetInputByIndex
    :project: helics


.. doxygenfunction:: helicsFederateGetInputCount
    :project: helics


.. doxygenfunction:: helicsFederateGetIntegerProperty
    :project: helics


.. doxygenfunction:: helicsFederateGetMessage
    :project: helics


.. doxygenfunction:: helicsFederateGetMessageObject
    :project: helics


.. doxygenfunction:: helicsFederateGetName
    :project: helics


.. doxygenfunction:: helicsFederateGetPublication
    :project: helics


.. doxygenfunction:: helicsFederateGetPublicationByIndex
    :project: helics


.. doxygenfunction:: helicsFederateGetPublicationCount
    :project: helics


.. doxygenfunction:: helicsFederateGetState
    :project: helics


.. doxygenfunction:: helicsFederateGetSubscription
    :project: helics


.. doxygenfunction:: helicsFederateGetTimeProperty
    :project: helics


.. doxygenfunction:: helicsFederateGlobalError
    :project: helics


.. doxygenfunction:: helicsFederateHasMessage
    :project: helics


.. doxygenfunction:: helicsFederateIsAsyncOperationCompleted
    :project: helics


.. doxygenfunction:: helicsFederateIsValid
    :project: helics


.. doxygenfunction:: helicsFederateLocalError
    :project: helics


.. doxygenfunction:: helicsFederateLogDebugMessage
    :project: helics


.. doxygenfunction:: helicsFederateLogErrorMessage
    :project: helics


.. doxygenfunction:: helicsFederateLogInfoMessage
    :project: helics


.. doxygenfunction:: helicsFederateLogLevelMessage
    :project: helics


.. doxygenfunction:: helicsFederateLogWarningMessage
    :project: helics


.. doxygenfunction:: helicsFederatePendingMessages
    :project: helics


.. doxygenfunction:: helicsFederatePublishJSON
    :project: helics


.. doxygenfunction:: helicsFederateRegisterCloningFilter
    :project: helics


.. doxygenfunction:: helicsFederateRegisterEndpoint
    :project: helics


.. doxygenfunction:: helicsFederateRegisterFilter
    :project: helics


.. doxygenfunction:: helicsFederateRegisterFromPublicationJSON
    :project: helics


.. doxygenfunction:: helicsFederateRegisterGlobalCloningFilter
    :project: helics


.. doxygenfunction:: helicsFederateRegisterGlobalEndpoint
    :project: helics


.. doxygenfunction:: helicsFederateRegisterGlobalFilter
    :project: helics


.. doxygenfunction:: helicsFederateRegisterGlobalInput
    :project: helics


.. doxygenfunction:: helicsFederateRegisterGlobalPublication
    :project: helics


.. doxygenfunction:: helicsFederateRegisterGlobalTypeInput
    :project: helics


.. doxygenfunction:: helicsFederateRegisterGlobalTypePublication
    :project: helics


.. doxygenfunction:: helicsFederateRegisterInput
    :project: helics


.. doxygenfunction:: helicsFederateRegisterInterfaces
    :project: helics


.. doxygenfunction:: helicsFederateRegisterPublication
    :project: helics


.. doxygenfunction:: helicsFederateRegisterSubscription
    :project: helics


.. doxygenfunction:: helicsFederateRegisterTypeInput
    :project: helics


.. doxygenfunction:: helicsFederateRegisterTypePublication
    :project: helics


.. doxygenfunction:: helicsFederateRequestNextStep
    :project: helics


.. doxygenfunction:: helicsFederateRequestTime
    :project: helics


.. doxygenfunction:: helicsFederateRequestTimeAdvance
    :project: helics


.. doxygenfunction:: helicsFederateRequestTimeAsync
    :project: helics


.. doxygenfunction:: helicsFederateRequestTimeComplete
    :project: helics


.. doxygenfunction:: helicsFederateRequestTimeIterative
    :project: helics


.. doxygenfunction:: helicsFederateRequestTimeIterativeAsync
    :project: helics


.. doxygenfunction:: helicsFederateRequestTimeIterativeComplete
    :project: helics


.. doxygenfunction:: helicsFederateSetFlagOption
    :project: helics


.. doxygenfunction:: helicsFederateSetGlobal
    :project: helics


.. doxygenfunction:: helicsFederateSetIntegerProperty
    :project: helics


.. doxygenfunction:: helicsFederateSetLogFile
    :project: helics


.. doxygenfunction:: helicsFederateSetSeparator
    :project: helics


.. doxygenfunction:: helicsFederateSetTimeProperty
    :project: helics


.. doxygenfunction:: helicsFederateEnterInitializingModeCompleted
    :project: helics


.. doxygenfunction:: helicsFederateSetLoggingCallback
    :project: helics

```

### Filter

```eval_rst

.. doxygenfunction:: helicsFilterAddDeliveryEndpoint
    :project: helics


.. doxygenfunction:: helicsFilterAddDestinationTarget
    :project: helics


.. doxygenfunction:: helicsFilterAddSourceTarget
    :project: helics


.. doxygenfunction:: helicsFilterGetInfo
    :project: helics


.. doxygenfunction:: helicsFilterGetName
    :project: helics


.. doxygenfunction:: helicsFilterGetOption
    :project: helics


.. doxygenfunction:: helicsFilterIsValid
    :project: helics


.. doxygenfunction:: helicsFilterRemoveDeliveryEndpoint
    :project: helics


.. doxygenfunction:: helicsFilterRemoveTarget
    :project: helics


.. doxygenfunction:: helicsFilterSet
    :project: helics


.. doxygenfunction:: helicsFilterSetInfo
    :project: helics


.. doxygenfunction:: helicsFilterSetOption
    :project: helics


.. doxygenfunction:: helicsFilterSetString
    :project: helics


.. doxygenfunction:: helicsFilterSetCustomCallback
    :project: helics

```

### Input

```eval_rst

.. doxygenfunction:: helicsInputAddTarget
    :project: helics


.. doxygenfunction:: helicsInputClearUpdate
    :project: helics


.. doxygenfunction:: helicsInputGetBoolean
    :project: helics


.. doxygenfunction:: helicsInputGetChar
    :project: helics


.. doxygenfunction:: helicsInputGetComplex
    :project: helics


.. doxygenfunction:: helicsInputGetComplexObject
    :project: helics


.. doxygenfunction:: helicsInputGetDouble
    :project: helics


.. doxygenfunction:: helicsInputGetExtractionUnits
    :project: helics


.. doxygenfunction:: helicsInputGetInfo
    :project: helics


.. doxygenfunction:: helicsInputGetInjectionUnits
    :project: helics


.. doxygenfunction:: helicsInputGetInteger
    :project: helics


.. doxygenfunction:: helicsInputGetKey
    :project: helics


.. doxygenfunction:: helicsInputGetNamedPoint
    :project: helics


.. doxygenfunction:: helicsInputGetOption
    :project: helics


.. doxygenfunction:: helicsInputGetPublicationType
    :project: helics


.. doxygenfunction:: helicsInputGetRawValue
    :project: helics


.. doxygenfunction:: helicsInputGetRawValueSize
    :project: helics


.. doxygenfunction:: helicsInputGetString
    :project: helics


.. doxygenfunction:: helicsInputGetStringSize
    :project: helics


.. doxygenfunction:: helicsInputGetTime
    :project: helics


.. doxygenfunction:: helicsInputGetType
    :project: helics


.. doxygenfunction:: helicsInputGetUnits
    :project: helics


.. doxygenfunction:: helicsInputGetVector
    :project: helics


.. doxygenfunction:: helicsInputGetVectorSize
    :project: helics


.. doxygenfunction:: helicsInputIsUpdated
    :project: helics


.. doxygenfunction:: helicsInputIsValid
    :project: helics


.. doxygenfunction:: helicsInputLastUpdateTime
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultBoolean
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultChar
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultComplex
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultDouble
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultInteger
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultNamedPoint
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultRaw
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultString
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultTime
    :project: helics


.. doxygenfunction:: helicsInputSetDefaultVector
    :project: helics


.. doxygenfunction:: helicsInputSetInfo
    :project: helics


.. doxygenfunction:: helicsInputSetMinimumChange
    :project: helics


.. doxygenfunction:: helicsInputSetOption
    :project: helics

```

### Message

```eval_rst

.. doxygenfunction:: helicsMessageAppendData
    :project: helics


.. doxygenfunction:: helicsMessageCheckFlag
    :project: helics


.. doxygenfunction:: helicsMessageClearFlags
    :project: helics


.. doxygenfunction:: helicsMessageClone
    :project: helics


.. doxygenfunction:: helicsMessageCopy
    :project: helics


.. doxygenfunction:: helicsMessageFree
    :project: helics


.. doxygenfunction:: helicsMessageGetDestination
    :project: helics


.. doxygenfunction:: helicsMessageGetMessageID
    :project: helics


.. doxygenfunction:: helicsMessageGetOriginalDestination
    :project: helics


.. doxygenfunction:: helicsMessageGetOriginalSource
    :project: helics


.. doxygenfunction:: helicsMessageGetRawData
    :project: helics


.. doxygenfunction:: helicsMessageGetRawDataSize
    :project: helics


.. doxygenfunction:: helicsMessageGetSource
    :project: helics


.. doxygenfunction:: helicsMessageGetString
    :project: helics


.. doxygenfunction:: helicsMessageGetTime
    :project: helics


.. doxygenfunction:: helicsMessageIsValid
    :project: helics


.. doxygenfunction:: helicsMessageReserve
    :project: helics


.. doxygenfunction:: helicsMessageSetData
    :project: helics


.. doxygenfunction:: helicsMessageSetDestination
    :project: helics


.. doxygenfunction:: helicsMessageSetFlagOption
    :project: helics


.. doxygenfunction:: helicsMessageSetMessageID
    :project: helics


.. doxygenfunction:: helicsMessageSetOriginalDestination
    :project: helics


.. doxygenfunction:: helicsMessageSetOriginalSource
    :project: helics


.. doxygenfunction:: helicsMessageSetSource
    :project: helics


.. doxygenfunction:: helicsMessageSetString
    :project: helics


.. doxygenfunction:: helicsMessageSetTime
    :project: helics


.. doxygenfunction:: helicsMessageGetRawDataPointer
    :project: helics


.. doxygenfunction:: helicsMessageSetOrginalSource
    :project: helics


.. doxygenfunction:: helicsMessageResize
    :project: helics

```

### Publication

```eval_rst

.. doxygenfunction:: helicsPublicationAddTarget
    :project: helics


.. doxygenfunction:: helicsPublicationGetInfo
    :project: helics


.. doxygenfunction:: helicsPublicationGetKey
    :project: helics


.. doxygenfunction:: helicsPublicationGetOption
    :project: helics


.. doxygenfunction:: helicsPublicationGetType
    :project: helics


.. doxygenfunction:: helicsPublicationGetUnits
    :project: helics


.. doxygenfunction:: helicsPublicationIsValid
    :project: helics


.. doxygenfunction:: helicsPublicationPublishBoolean
    :project: helics


.. doxygenfunction:: helicsPublicationPublishChar
    :project: helics


.. doxygenfunction:: helicsPublicationPublishComplex
    :project: helics


.. doxygenfunction:: helicsPublicationPublishDouble
    :project: helics


.. doxygenfunction:: helicsPublicationPublishInteger
    :project: helics


.. doxygenfunction:: helicsPublicationPublishNamedPoint
    :project: helics


.. doxygenfunction:: helicsPublicationPublishRaw
    :project: helics


.. doxygenfunction:: helicsPublicationPublishString
    :project: helics


.. doxygenfunction:: helicsPublicationPublishTime
    :project: helics


.. doxygenfunction:: helicsPublicationPublishVector
    :project: helics


.. doxygenfunction:: helicsPublicationSetInfo
    :project: helics


.. doxygenfunction:: helicsPublicationSetMinimumChange
    :project: helics


.. doxygenfunction:: helicsPublicationSetOption
    :project: helics

```

### Query

```eval_rst

.. doxygenfunction:: helicsQueryBrokerExecute
    :project: helics


.. doxygenfunction:: helicsQueryCoreExecute
    :project: helics


.. doxygenfunction:: helicsQueryExecute
    :project: helics


.. doxygenfunction:: helicsQueryExecuteAsync
    :project: helics


.. doxygenfunction:: helicsQueryExecuteComplete
    :project: helics


.. doxygenfunction:: helicsQueryFree
    :project: helics


.. doxygenfunction:: helicsQueryIsCompleted
    :project: helics


.. doxygenfunction:: helicsQueryExecuteCompleted
    :project: helics


.. doxygenfunction:: helicsQuerySetTarget
    :project: helics


.. doxygenfunction:: helicsQuerySetQueryString
    :project: helics

```

### Others

```eval_rst

.. doxygenfunction:: helicsCleanupLibrary
    :project: helics


.. doxygenfunction:: helicsCloseLibrary
    :project: helics


.. doxygenfunction:: helicsCreateBroker
    :project: helics


.. doxygenfunction:: helicsCreateBrokerFromArgs
    :project: helics


.. doxygenfunction:: helicsCreateCombinationFederate
    :project: helics


.. doxygenfunction:: helicsCreateCombinationFederateFromConfig
    :project: helics


.. doxygenfunction:: helicsCreateCore
    :project: helics


.. doxygenfunction:: helicsCreateCoreFromArgs
    :project: helics


.. doxygenfunction:: helicsCreateFederateInfo
    :project: helics


.. doxygenfunction:: helicsCreateMessageFederate
    :project: helics


.. doxygenfunction:: helicsCreateMessageFederateFromConfig
    :project: helics


.. doxygenfunction:: helicsCreateQuery
    :project: helics


.. doxygenfunction:: helicsCreateValueFederate
    :project: helics


.. doxygenfunction:: helicsCreateValueFederateFromConfig
    :project: helics


.. doxygenfunction:: helicsGetBuildFlags
    :project: helics


.. doxygenfunction:: helicsGetCompilerVersion
    :project: helics


.. doxygenfunction:: helicsGetFederateByName
    :project: helics


.. doxygenfunction:: helicsGetOptionIndex
    :project: helics


.. doxygenfunction:: helicsGetPropertyIndex
    :project: helics


.. doxygenfunction:: helicsGetVersion
    :project: helics


.. doxygenfunction:: helicsIsCoreTypeAvailable
    :project: helics


.. doxygenfunction:: helicsSubscriptionGetKey
    :project: helics


.. doxygenfunction:: helicsErrorInitialize
    :project: helics


.. doxygenfunction:: helicsErrorClear
    :project: helics

```
