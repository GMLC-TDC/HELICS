
# C API Reference



```eval_rst

.. doxygendefine:: helicsBrokerAddDestinationFilterToEndpoint
    :project: helics


.. doxygendefine:: helicsBrokerAddSourceFilterToEndpoint
    :project: helics


.. doxygendefine:: helicsBrokerClone
    :project: helics


.. doxygendefine:: helicsBrokerDataLink
    :project: helics


.. doxygendefine:: helicsBrokerDestroy
    :project: helics


.. doxygendefine:: helicsBrokerDisconnect
    :project: helics


.. doxygendefine:: helicsBrokerFree
    :project: helics


.. doxygendefine:: helicsBrokerGetAddress
    :project: helics


.. doxygendefine:: helicsBrokerGetIdentifier
    :project: helics


.. doxygendefine:: helicsBrokerIsConnected
    :project: helics


.. doxygendefine:: helicsBrokerIsValid
    :project: helics


.. doxygendefine:: helicsBrokerSetGlobal
    :project: helics


.. doxygendefine:: helicsBrokerWaitForDisconnect
    :project: helics


.. doxygendefine:: helicsCleanupLibrary
    :project: helics


.. doxygendefine:: helicsCloseLibrary
    :project: helics


.. doxygendefine:: helicsCoreAddDestinationFilterToEndpoint
    :project: helics


.. doxygendefine:: helicsCoreAddSourceFilterToEndpoint
    :project: helics


.. doxygendefine:: helicsCoreClone
    :project: helics


.. doxygendefine:: helicsCoreDataLink
    :project: helics


.. doxygendefine:: helicsCoreDestroy
    :project: helics


.. doxygendefine:: helicsCoreDisconnect
    :project: helics


.. doxygendefine:: helicsCoreFree
    :project: helics


.. doxygendefine:: helicsCoreGetIdentifier
    :project: helics


.. doxygendefine:: helicsCoreIsConnected
    :project: helics


.. doxygendefine:: helicsCoreIsValid
    :project: helics


.. doxygendefine:: helicsCoreRegisterCloningFilter
    :project: helics


.. doxygendefine:: helicsCoreRegisterFilter
    :project: helics


.. doxygendefine:: helicsCoreSetGlobal
    :project: helics


.. doxygendefine:: helicsCoreSetReadyToInit
    :project: helics


.. doxygendefine:: helicsCreateBroker
    :project: helics


.. doxygendefine:: helicsCreateBrokerFromArgs
    :project: helics


.. doxygendefine:: helicsCreateCombinationFederate
    :project: helics


.. doxygendefine:: helicsCreateCombinationFederateFromConfig
    :project: helics


.. doxygendefine:: helicsCreateCore
    :project: helics


.. doxygendefine:: helicsCreateCoreFromArgs
    :project: helics


.. doxygendefine:: helicsCreateFederateInfo
    :project: helics


.. doxygendefine:: helicsCreateMessageFederate
    :project: helics


.. doxygendefine:: helicsCreateMessageFederateFromConfig
    :project: helics


.. doxygendefine:: helicsCreateQuery
    :project: helics


.. doxygendefine:: helicsCreateValueFederate
    :project: helics


.. doxygendefine:: helicsCreateValueFederateFromConfig
    :project: helics


.. doxygendefine:: helicsEndpointGetDefaultDestination
    :project: helics


.. doxygendefine:: helicsEndpointGetInfo
    :project: helics


.. doxygendefine:: helicsEndpointGetMessage
    :project: helics


.. doxygendefine:: helicsEndpointGetName
    :project: helics


.. doxygendefine:: helicsEndpointGetOption
    :project: helics


.. doxygendefine:: helicsEndpointGetType
    :project: helics


.. doxygendefine:: helicsEndpointHasMessage
    :project: helics


.. doxygendefine:: helicsEndpointPendingMessages
    :project: helics


.. doxygendefine:: helicsEndpointSendEventRaw
    :project: helics


.. doxygendefine:: helicsEndpointSendMessage
    :project: helics


.. doxygendefine:: helicsEndpointSendMessageRaw
    :project: helics


.. doxygendefine:: helicsEndpointSetDefaultDestination
    :project: helics


.. doxygendefine:: helicsEndpointSetInfo
    :project: helics


.. doxygendefine:: helicsEndpointSetOption
    :project: helics


.. doxygendefine:: helicsEndpointSubscribe
    :project: helics


.. doxygendefine:: helicsFederateClone
    :project: helics


.. doxygendefine:: helicsFederateDestroy
    :project: helics


.. doxygendefine:: helicsFederateEnterExecutingMode
    :project: helics


.. doxygendefine:: helicsFederateEnterExecutingModeAsync
    :project: helics


.. doxygendefine:: helicsFederateEnterExecutingModeComplete
    :project: helics


.. doxygendefine:: helicsFederateEnterExecutingModeIterative
    :project: helics


.. doxygendefine:: helicsFederateEnterExecutingModeIterativeAsync
    :project: helics


.. doxygendefine:: helicsFederateEnterExecutingModeIterativeComplete
    :project: helics


.. doxygendefine:: helicsFederateEnterInitializingMode
    :project: helics


.. doxygendefine:: helicsFederateEnterInitializingModeAsync
    :project: helics


.. doxygendefine:: helicsFederateEnterInitializingModeComplete
    :project: helics


.. doxygendefine:: helicsFederateFinalize
    :project: helics


.. doxygendefine:: helicsFederateFinalizeAsync
    :project: helics


.. doxygendefine:: helicsFederateFinalizeComplete
    :project: helics


.. doxygendefine:: helicsFederateFree
    :project: helics


.. doxygendefine:: helicsFederateGetCoreObject
    :project: helics


.. doxygendefine:: helicsFederateGetCurrentTime
    :project: helics


.. doxygendefine:: helicsFederateGetEndpoint
    :project: helics


.. doxygendefine:: helicsFederateGetEndpointByIndex
    :project: helics


.. doxygendefine:: helicsFederateGetEndpointCount
    :project: helics


.. doxygendefine:: helicsFederateGetFilter
    :project: helics


.. doxygendefine:: helicsFederateGetFilterByIndex
    :project: helics


.. doxygendefine:: helicsFederateGetFilterCount
    :project: helics


.. doxygendefine:: helicsFederateGetFlagOption
    :project: helics


.. doxygendefine:: helicsFederateGetInput
    :project: helics


.. doxygendefine:: helicsFederateGetInputByIndex
    :project: helics


.. doxygendefine:: helicsFederateGetInputCount
    :project: helics


.. doxygendefine:: helicsFederateGetIntegerProperty
    :project: helics


.. doxygendefine:: helicsFederateGetMessage
    :project: helics


.. doxygendefine:: helicsFederateGetName
    :project: helics


.. doxygendefine:: helicsFederateGetPublication
    :project: helics


.. doxygendefine:: helicsFederateGetPublicationByIndex
    :project: helics


.. doxygendefine:: helicsFederateGetPublicationCount
    :project: helics


.. doxygendefine:: helicsFederateGetState
    :project: helics


.. doxygendefine:: helicsFederateGetSubscription
    :project: helics


.. doxygendefine:: helicsFederateGetTimeProperty
    :project: helics


.. doxygendefine:: helicsFederateHasMessage
    :project: helics


.. doxygendefine:: helicsFederateInfoClone
    :project: helics


.. doxygendefine:: helicsFederateInfoFree
    :project: helics


.. doxygendefine:: helicsFederateInfoLoadFromArgs
    :project: helics


.. doxygendefine:: helicsFederateInfoSetBroker
    :project: helics


.. doxygendefine:: helicsFederateInfoSetBrokerPort
    :project: helics


.. doxygendefine:: helicsFederateInfoSetCoreInitString
    :project: helics


.. doxygendefine:: helicsFederateInfoSetCoreName
    :project: helics


.. doxygendefine:: helicsFederateInfoSetCoreType
    :project: helics


.. doxygendefine:: helicsFederateInfoSetCoreTypeFromString
    :project: helics


.. doxygendefine:: helicsFederateInfoSetFlagOption
    :project: helics


.. doxygendefine:: helicsFederateInfoSetIntegerProperty
    :project: helics


.. doxygendefine:: helicsFederateInfoSetLocalPort
    :project: helics


.. doxygendefine:: helicsFederateInfoSetSeparator
    :project: helics


.. doxygendefine:: helicsFederateInfoSetTimeProperty
    :project: helics


.. doxygendefine:: helicsFederateIsAsyncOperationCompleted
    :project: helics


.. doxygendefine:: helicsFederateIsValid
    :project: helics


.. doxygendefine:: helicsFederatePendingMessages
    :project: helics


.. doxygendefine:: helicsFederateRegisterCloningFilter
    :project: helics


.. doxygendefine:: helicsFederateRegisterEndpoint
    :project: helics


.. doxygendefine:: helicsFederateRegisterFilter
    :project: helics


.. doxygendefine:: helicsFederateRegisterGlobalCloningFilter
    :project: helics


.. doxygendefine:: helicsFederateRegisterGlobalEndpoint
    :project: helics


.. doxygendefine:: helicsFederateRegisterGlobalFilter
    :project: helics


.. doxygendefine:: helicsFederateRegisterGlobalInput
    :project: helics


.. doxygendefine:: helicsFederateRegisterGlobalPublication
    :project: helics


.. doxygendefine:: helicsFederateRegisterGlobalTypeInput
    :project: helics


.. doxygendefine:: helicsFederateRegisterGlobalTypePublication
    :project: helics


.. doxygendefine:: helicsFederateRegisterInput
    :project: helics


.. doxygendefine:: helicsFederateRegisterInterfaces
    :project: helics


.. doxygendefine:: helicsFederateRegisterPublication
    :project: helics


.. doxygendefine:: helicsFederateRegisterSubscription
    :project: helics


.. doxygendefine:: helicsFederateRegisterTypeInput
    :project: helics


.. doxygendefine:: helicsFederateRegisterTypePublication
    :project: helics


.. doxygendefine:: helicsFederateRequestNextStep
    :project: helics


.. doxygendefine:: helicsFederateRequestTime
    :project: helics


.. doxygendefine:: helicsFederateRequestTimeAsync
    :project: helics


.. doxygendefine:: helicsFederateRequestTimeComplete
    :project: helics


.. doxygendefine:: helicsFederateRequestTimeIterative
    :project: helics


.. doxygendefine:: helicsFederateRequestTimeIterativeAsync
    :project: helics


.. doxygendefine:: helicsFederateRequestTimeIterativeComplete
    :project: helics


.. doxygendefine:: helicsFederateSetFlagOption
    :project: helics


.. doxygendefine:: helicsFederateSetGlobal
    :project: helics


.. doxygendefine:: helicsFederateSetIntegerProperty
    :project: helics


.. doxygendefine:: helicsFederateSetSeparator
    :project: helics


.. doxygendefine:: helicsFederateSetTimeProperty
    :project: helics


.. doxygendefine:: helicsFilterAddDeliveryEndpoint
    :project: helics


.. doxygendefine:: helicsFilterAddDestinationTarget
    :project: helics


.. doxygendefine:: helicsFilterAddSourceTarget
    :project: helics


.. doxygendefine:: helicsFilterGetInfo
    :project: helics


.. doxygendefine:: helicsFilterGetName
    :project: helics


.. doxygendefine:: helicsFilterGetOption
    :project: helics


.. doxygendefine:: helicsFilterRemoveDeliveryEndpoint
    :project: helics


.. doxygendefine:: helicsFilterRemoveTarget
    :project: helics


.. doxygendefine:: helicsFilterSet
    :project: helics


.. doxygendefine:: helicsFilterSetInfo
    :project: helics


.. doxygendefine:: helicsFilterSetOption
    :project: helics


.. doxygendefine:: helicsFilterSetString
    :project: helics


.. doxygendefine:: helicsGetFederateByName
    :project: helics


.. doxygendefine:: helicsGetOptionIndex
    :project: helics


.. doxygendefine:: helicsGetPropertyIndex
    :project: helics


.. doxygendefine:: helicsGetVersion
    :project: helics


.. doxygendefine:: helicsInputAddTarget
    :project: helics


.. doxygendefine:: helicsInputGetBoolean
    :project: helics


.. doxygendefine:: helicsInputGetChar
    :project: helics


.. doxygendefine:: helicsInputGetComplex
    :project: helics


.. doxygendefine:: helicsInputGetComplexObject
    :project: helics


.. doxygendefine:: helicsInputGetDouble
    :project: helics


.. doxygendefine:: helicsInputGetInfo
    :project: helics


.. doxygendefine:: helicsInputGetInteger
    :project: helics


.. doxygendefine:: helicsInputGetKey
    :project: helics


.. doxygendefine:: helicsInputGetNamedPoint
    :project: helics


.. doxygendefine:: helicsInputGetOption
    :project: helics


.. doxygendefine:: helicsInputGetPublicationType
    :project: helics


.. doxygendefine:: helicsInputGetRawValue
    :project: helics


.. doxygendefine:: helicsInputGetRawValueSize
    :project: helics


.. doxygendefine:: helicsInputGetString
    :project: helics


.. doxygendefine:: helicsInputGetStringSize
    :project: helics


.. doxygendefine:: helicsInputGetTime
    :project: helics


.. doxygendefine:: helicsInputGetType
    :project: helics


.. doxygendefine:: helicsInputGetUnits
    :project: helics


.. doxygendefine:: helicsInputGetVector
    :project: helics


.. doxygendefine:: helicsInputGetVectorSize
    :project: helics


.. doxygendefine:: helicsInputIsUpdated
    :project: helics


.. doxygendefine:: helicsInputLastUpdateTime
    :project: helics


.. doxygendefine:: helicsInputSetDefaultBoolean
    :project: helics


.. doxygendefine:: helicsInputSetDefaultChar
    :project: helics


.. doxygendefine:: helicsInputSetDefaultComplex
    :project: helics


.. doxygendefine:: helicsInputSetDefaultDouble
    :project: helics


.. doxygendefine:: helicsInputSetDefaultInteger
    :project: helics


.. doxygendefine:: helicsInputSetDefaultNamedPoint
    :project: helics


.. doxygendefine:: helicsInputSetDefaultRaw
    :project: helics


.. doxygendefine:: helicsInputSetDefaultString
    :project: helics


.. doxygendefine:: helicsInputSetDefaultTime
    :project: helics


.. doxygendefine:: helicsInputSetDefaultVector
    :project: helics


.. doxygendefine:: helicsInputSetInfo
    :project: helics


.. doxygendefine:: helicsInputSetOption
    :project: helics


.. doxygendefine:: helicsIsCoreTypeAvailable
    :project: helics


.. doxygendefine:: helicsPublicationAddTarget
    :project: helics


.. doxygendefine:: helicsPublicationGetInfo
    :project: helics


.. doxygendefine:: helicsPublicationGetKey
    :project: helics


.. doxygendefine:: helicsPublicationGetOption
    :project: helics


.. doxygendefine:: helicsPublicationGetType
    :project: helics


.. doxygendefine:: helicsPublicationGetUnits
    :project: helics


.. doxygendefine:: helicsPublicationPublishBoolean
    :project: helics


.. doxygendefine:: helicsPublicationPublishChar
    :project: helics


.. doxygendefine:: helicsPublicationPublishComplex
    :project: helics


.. doxygendefine:: helicsPublicationPublishDouble
    :project: helics


.. doxygendefine:: helicsPublicationPublishInteger
    :project: helics


.. doxygendefine:: helicsPublicationPublishNamedPoint
    :project: helics


.. doxygendefine:: helicsPublicationPublishRaw
    :project: helics


.. doxygendefine:: helicsPublicationPublishString
    :project: helics


.. doxygendefine:: helicsPublicationPublishTime
    :project: helics


.. doxygendefine:: helicsPublicationPublishVector
    :project: helics


.. doxygendefine:: helicsPublicationSetInfo
    :project: helics


.. doxygendefine:: helicsPublicationSetOption
    :project: helics


.. doxygendefine:: helicsQueryBrokerExecute
    :project: helics


.. doxygendefine:: helicsQueryCoreExecute
    :project: helics


.. doxygendefine:: helicsQueryExecute
    :project: helics


.. doxygendefine:: helicsQueryExecuteAsync
    :project: helics


.. doxygendefine:: helicsQueryExecuteComplete
    :project: helics


.. doxygendefine:: helicsQueryFree
    :project: helics


.. doxygendefine:: helicsQueryIsCompleted
    :project: helics


.. doxygendefine:: helicsSubscriptionGetKey
    :project: helics


.. doxygendefine:: helics_complex
    :project: helics


.. doxygendefine:: helics_complex_swigregister
    :project: helics


.. doxygendefine:: helics_core_type_default
    :project: helics


.. doxygendefine:: helics_core_type_http
    :project: helics


.. doxygendefine:: helics_core_type_interprocess
    :project: helics


.. doxygendefine:: helics_core_type_ipc
    :project: helics


.. doxygendefine:: helics_core_type_mpi
    :project: helics


.. doxygendefine:: helics_core_type_nng
    :project: helics


.. doxygendefine:: helics_core_type_tcp
    :project: helics


.. doxygendefine:: helics_core_type_tcp_ss
    :project: helics


.. doxygendefine:: helics_core_type_test
    :project: helics


.. doxygendefine:: helics_core_type_udp
    :project: helics


.. doxygendefine:: helics_core_type_zmq
    :project: helics


.. doxygendefine:: helics_core_type_zmq_test
    :project: helics


.. doxygendefine:: helics_data_type_any
    :project: helics


.. doxygendefine:: helics_data_type_boolean
    :project: helics


.. doxygendefine:: helics_data_type_complex
    :project: helics


.. doxygendefine:: helics_data_type_complex_vector
    :project: helics


.. doxygendefine:: helics_data_type_double
    :project: helics


.. doxygendefine:: helics_data_type_int
    :project: helics


.. doxygendefine:: helics_data_type_named_point
    :project: helics


.. doxygendefine:: helics_data_type_raw
    :project: helics


.. doxygendefine:: helics_data_type_string
    :project: helics


.. doxygendefine:: helics_data_type_time
    :project: helics


.. doxygendefine:: helics_data_type_vector
    :project: helics


.. doxygendefine:: helics_error_connection_failure
    :project: helics


.. doxygendefine:: helics_error_discard
    :project: helics


.. doxygendefine:: helics_error_execution_failure
    :project: helics


.. doxygendefine:: helics_error_invalid_argument
    :project: helics


.. doxygendefine:: helics_error_invalid_function_call
    :project: helics


.. doxygendefine:: helics_error_invalid_object
    :project: helics


.. doxygendefine:: helics_error_invalid_state_transition
    :project: helics


.. doxygendefine:: helics_error_other
    :project: helics


.. doxygendefine:: helics_error_registration_failure
    :project: helics


.. doxygendefine:: helics_error_system_failure
    :project: helics


.. doxygendefine:: helics_false
    :project: helics


.. doxygendefine:: helics_filter_type_clone
    :project: helics


.. doxygendefine:: helics_filter_type_custom
    :project: helics


.. doxygendefine:: helics_filter_type_delay
    :project: helics


.. doxygendefine:: helics_filter_type_firewall
    :project: helics


.. doxygendefine:: helics_filter_type_random_delay
    :project: helics


.. doxygendefine:: helics_filter_type_random_drop
    :project: helics


.. doxygendefine:: helics_filter_type_reroute
    :project: helics


.. doxygendefine:: helics_flag_delay_init_entry
    :project: helics


.. doxygendefine:: helics_flag_enable_init_entry
    :project: helics


.. doxygendefine:: helics_flag_forward_compute
    :project: helics


.. doxygendefine:: helics_flag_ignore_time_mismatch_warnings
    :project: helics


.. doxygendefine:: helics_flag_interruptible
    :project: helics


.. doxygendefine:: helics_flag_observer
    :project: helics


.. doxygendefine:: helics_flag_only_transmit_on_change
    :project: helics


.. doxygendefine:: helics_flag_only_update_on_change
    :project: helics


.. doxygendefine:: helics_flag_realtime
    :project: helics


.. doxygendefine:: helics_flag_rollback
    :project: helics


.. doxygendefine:: helics_flag_single_thread_federate
    :project: helics


.. doxygendefine:: helics_flag_source_only
    :project: helics


.. doxygendefine:: helics_flag_uninterruptible
    :project: helics


.. doxygendefine:: helics_flag_wait_for_current_time_update
    :project: helics


.. doxygendefine:: helics_handle_option_buffer_data
    :project: helics


.. doxygendefine:: helics_handle_option_connection_optional
    :project: helics


.. doxygendefine:: helics_handle_option_connection_required
    :project: helics


.. doxygendefine:: helics_handle_option_ignore_interrupts
    :project: helics


.. doxygendefine:: helics_handle_option_multiple_connections_allowed
    :project: helics


.. doxygendefine:: helics_handle_option_only_transmit_on_change
    :project: helics


.. doxygendefine:: helics_handle_option_only_update_on_change
    :project: helics


.. doxygendefine:: helics_handle_option_single_connection_only
    :project: helics


.. doxygendefine:: helics_handle_option_strict_type_checking
    :project: helics


.. doxygendefine:: helics_iteration_request_force_iteration
    :project: helics


.. doxygendefine:: helics_iteration_request_iterate_if_needed
    :project: helics


.. doxygendefine:: helics_iteration_request_no_iteration
    :project: helics


.. doxygendefine:: helics_iteration_result_error
    :project: helics


.. doxygendefine:: helics_iteration_result_halted
    :project: helics


.. doxygendefine:: helics_iteration_result_iterating
    :project: helics


.. doxygendefine:: helics_iteration_result_next_step
    :project: helics


.. doxygendefine:: helics_log_level_connections
    :project: helics


.. doxygendefine:: helics_log_level_data
    :project: helics


.. doxygendefine:: helics_log_level_error
    :project: helics


.. doxygendefine:: helics_log_level_interfaces
    :project: helics


.. doxygendefine:: helics_log_level_no_print
    :project: helics


.. doxygendefine:: helics_log_level_summary
    :project: helics


.. doxygendefine:: helics_log_level_timing
    :project: helics


.. doxygendefine:: helics_log_level_trace
    :project: helics


.. doxygendefine:: helics_log_level_warning
    :project: helics


.. doxygendefine:: helics_message
    :project: helics


.. doxygendefine:: helics_message_swigregister
    :project: helics


.. doxygendefine:: helics_ok
    :project: helics


.. doxygendefine:: helics_property_int_log_level
    :project: helics


.. doxygendefine:: helics_property_int_max_iterations
    :project: helics


.. doxygendefine:: helics_property_time_delta
    :project: helics


.. doxygendefine:: helics_property_time_input_delay
    :project: helics


.. doxygendefine:: helics_property_time_offset
    :project: helics


.. doxygendefine:: helics_property_time_output_delay
    :project: helics


.. doxygendefine:: helics_property_time_period
    :project: helics


.. doxygendefine:: helics_property_time_rt_lag
    :project: helics


.. doxygendefine:: helics_property_time_rt_lead
    :project: helics


.. doxygendefine:: helics_property_time_rt_tolerance
    :project: helics


.. doxygendefine:: helics_state_error
    :project: helics


.. doxygendefine:: helics_state_execution
    :project: helics


.. doxygendefine:: helics_state_finalize
    :project: helics


.. doxygendefine:: helics_state_initialization
    :project: helics


.. doxygendefine:: helics_state_pending_exec
    :project: helics


.. doxygendefine:: helics_state_pending_finalize
    :project: helics


.. doxygendefine:: helics_state_pending_init
    :project: helics


.. doxygendefine:: helics_state_pending_iterative_time
    :project: helics


.. doxygendefine:: helics_state_pending_time
    :project: helics


.. doxygendefine:: helics_state_startup
    :project: helics


.. doxygendefine:: helics_time_epsilon
    :project: helics


.. doxygendefine:: helics_time_invalid
    :project: helics


.. doxygendefine:: helics_time_maxtime
    :project: helics


.. doxygendefine:: helics_time_zero
    :project: helics


.. doxygendefine:: helics_true
    :project: helics


.. doxygendefine:: helics_warning
    :project: helics


.. doxygendefine:: other_error_type
    :project: helics

```
