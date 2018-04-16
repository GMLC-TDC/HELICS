
# C API Reference



```eval_rst
.. doxygendefine:: HELICS_CORE_TYPE_DEFAULT
   :project: helics

.. doxygendefine:: HELICS_CORE_TYPE_INTERPROCESS
   :project: helics

.. doxygendefine:: HELICS_CORE_TYPE_IPC
   :project: helics

.. doxygendefine:: HELICS_CORE_TYPE_MPI
   :project: helics

.. doxygendefine:: HELICS_CORE_TYPE_TCP
   :project: helics

.. doxygendefine:: HELICS_CORE_TYPE_TEST
   :project: helics

.. doxygendefine:: HELICS_CORE_TYPE_UDP
   :project: helics

.. doxygendefine:: HELICS_CORE_TYPE_ZMQ
   :project: helics

.. doxygendefine:: HELICS_DATA_TYPE_COMPLEX
   :project: helics

.. doxygendefine:: HELICS_DATA_TYPE_DOUBLE
   :project: helics

.. doxygendefine:: HELICS_DATA_TYPE_INT
   :project: helics

.. doxygendefine:: HELICS_DATA_TYPE_RAW
   :project: helics

.. doxygendefine:: HELICS_DATA_TYPE_STRING
   :project: helics

.. doxygendefine:: HELICS_DATA_TYPE_VECTOR
   :project: helics

.. doxygenenumvalue:: force_iteration
   :project: helics

.. doxygenfunction:: helicsBrokerClone
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

.. doxygenfunction:: helicsCleanupHelicsLibrary
   :project: helics

.. doxygenfunction:: helicsCloseLibrary
   :project: helics

.. doxygenfunction:: helicsCoreClone
   :project: helics

.. doxygenfunction:: helicsCoreDisconnect
   :project: helics

.. doxygenfunction:: helicsCoreFree
   :project: helics

.. doxygenfunction:: helicsCoreGetIdentifier
   :project: helics

.. doxygenfunction:: helicsCoreIsConnected
   :project: helics

.. doxygenfunction:: helicsCoreRegisterCloningFilter
   :project: helics

.. doxygenfunction:: helicsCoreRegisterDestinationFilter
   :project: helics

.. doxygenfunction:: helicsCoreRegisterSourceFilter
   :project: helics

.. doxygenfunction:: helicsCoreSetReadyToInit
   :project: helics

.. doxygenfunction:: helicsCreateBroker
   :project: helics

.. doxygenfunction:: helicsCreateBrokerFromArgs
   :project: helics

.. doxygenfunction:: helicsCreateCombinationFederate
   :project: helics

.. doxygenfunction:: helicsCreateCombinationFederateFromJson
   :project: helics

.. doxygenfunction:: helicsCreateCore
   :project: helics

.. doxygenfunction:: helicsCreateCoreFromArgs
   :project: helics

.. doxygenfunction:: helicsCreateMessageFederate
   :project: helics

.. doxygenfunction:: helicsCreateMessageFederateFromJson
   :project: helics

.. doxygenfunction:: helicsCreateQuery
   :project: helics

.. doxygenfunction:: helicsCreateValueFederate
   :project: helics

.. doxygenfunction:: helicsCreateValueFederateFromJson
   :project: helics

.. doxygenfunction:: helicsEndpointGetMessage
   :project: helics

.. doxygenfunction:: helicsEndpointGetName
   :project: helics

.. doxygenfunction:: helicsEndpointGetType
   :project: helics

.. doxygenfunction:: helicsEndpointHasMessage
   :project: helics

.. doxygenfunction:: helicsEndpointReceiveCount
   :project: helics

.. doxygenfunction:: helicsEndpointSendEventRaw
   :project: helics

.. doxygenfunction:: helicsEndpointSendMessage
   :project: helics

.. doxygenfunction:: helicsEndpointSendMessageRaw
   :project: helics

.. doxygenfunction:: helicsEndpointSetDefaultDestination
   :project: helics

.. doxygenfunction:: helicsEndpointSubscribe
   :project: helics

.. doxygenfunction:: helicsFederateClone
   :project: helics

.. doxygenfunction:: helicsFederateEnterExecutionMode
   :project: helics

.. doxygenfunction:: helicsFederateEnterExecutionModeAsync
   :project: helics

.. doxygenfunction:: helicsFederateEnterExecutionModeComplete
   :project: helics

.. doxygenfunction:: helicsFederateEnterExecutionModeIterative
   :project: helics

.. doxygenfunction:: helicsFederateEnterExecutionModeIterativeAsync
   :project: helics

.. doxygenfunction:: helicsFederateEnterExecutionModeIterativeComplete
   :project: helics

.. doxygenfunction:: helicsFederateEnterInitializationMode
   :project: helics

.. doxygenfunction:: helicsFederateEnterInitializationModeAsync
   :project: helics

.. doxygenfunction:: helicsFederateEnterInitializationModeComplete
   :project: helics

.. doxygenfunction:: helicsFederateFinalize
   :project: helics

.. doxygenfunction:: helicsFederateFree
   :project: helics

.. doxygenfunction:: helicsFederateGetCoreObject
   :project: helics

.. doxygenfunction:: helicsFederateGetCurrentTime
   :project: helics

.. doxygenfunction:: helicsFederateGetEndpointCount
   :project: helics

.. doxygenfunction:: helicsFederateGetMessage
   :project: helics

.. doxygenfunction:: helicsFederateGetName
   :project: helics

.. doxygenfunction:: helicsFederateGetPublicationCount
   :project: helics

.. doxygenfunction:: helicsFederateGetState
   :project: helics

.. doxygenfunction:: helicsFederateGetSubscriptionCount
   :project: helics

.. doxygenfunction:: helicsFederateHasMessage
   :project: helics

.. doxygenfunction:: helicsFederateInfoCreate
   :project: helics

.. doxygenfunction:: helicsFederateInfoFree
   :project: helics

.. doxygenfunction:: helicsFederateInfoLoadFromArgs
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetCoreInitString
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetCoreName
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetCoreType
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetCoreTypeFromString
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetFederateName
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetFlag
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetInputDelay
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetLoggingLevel
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetMaxIterations
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetOutputDelay
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetPeriod
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetTimeDelta
   :project: helics

.. doxygenfunction:: helicsFederateInfoSetTimeOffset
   :project: helics

.. doxygenfunction:: helicsFederateIsAsyncOperationCompleted
   :project: helics

.. doxygenfunction:: helicsFederateReceiveCount
   :project: helics

.. doxygenfunction:: helicsFederateRegisterCloningFilter
   :project: helics

.. doxygenfunction:: helicsFederateRegisterDestinationFilter
   :project: helics

.. doxygenfunction:: helicsFederateRegisterEndpoint
   :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalEndpoint
   :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalPublication
   :project: helics

.. doxygenfunction:: helicsFederateRegisterGlobalTypePublication
   :project: helics

.. doxygenfunction:: helicsFederateRegisterOptionalSubscription
   :project: helics

.. doxygenfunction:: helicsFederateRegisterOptionalTypeSubscription
   :project: helics

.. doxygenfunction:: helicsFederateRegisterPublication
   :project: helics

.. doxygenfunction:: helicsFederateRegisterSourceFilter
   :project: helics

.. doxygenfunction:: helicsFederateRegisterSubscription
   :project: helics

.. doxygenfunction:: helicsFederateRegisterTypePublication
   :project: helics

.. doxygenfunction:: helicsFederateRegisterTypeSubscription
   :project: helics

.. doxygenfunction:: helicsFederateRequestTime
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

.. doxygenfunction:: helicsFederateSetFlag
   :project: helics

.. doxygenfunction:: helicsFederateSetInputDelay
   :project: helics

.. doxygenfunction:: helicsFederateSetLoggingLevel
   :project: helics

.. doxygenfunction:: helicsFederateSetMaxIterations
   :project: helics

.. doxygenfunction:: helicsFederateSetOutputDelay
   :project: helics

.. doxygenfunction:: helicsFederateSetPeriod
   :project: helics

.. doxygenfunction:: helicsFederateSetTimeDelta
   :project: helics

.. doxygenfunction:: helicsFilterAddDeliveryEndpoint
   :project: helics

.. doxygenfunction:: helicsFilterAddDestinationTarget
   :project: helics

.. doxygenfunction:: helicsFilterAddSourceTarget
   :project: helics

.. doxygenfunction:: helicsFilterGetName
   :project: helics

.. doxygenfunction:: helicsFilterGetTarget
   :project: helics

.. doxygenfunction:: helicsFilterRemoveDeliveryEndpoint
   :project: helics

.. doxygenfunction:: helicsFilterRemoveDestinationTarget
   :project: helics

.. doxygenfunction:: helicsFilterRemoveSourceTarget
   :project: helics

.. doxygenfunction:: helicsFilterSet
   :project: helics

.. doxygenfunction:: helicsFilterSetString
   :project: helics

.. doxygenfunction:: helicsGetFederateByName
   :project: helics

.. doxygenfunction:: helicsGetVersion
   :project: helics

.. doxygenfunction:: helicsIsCoreTypeAvailable
   :project: helics

.. doxygenfunction:: helicsPublicationGetKey
   :project: helics

.. doxygenfunction:: helicsPublicationGetType
   :project: helics

.. doxygenfunction:: helicsPublicationGetUnits
   :project: helics

.. doxygenfunction:: helicsPublicationPublishComplex
   :project: helics

.. doxygenfunction:: helicsPublicationPublishDouble
   :project: helics

.. doxygenfunction:: helicsPublicationPublishInteger
   :project: helics

.. doxygenfunction:: helicsPublicationPublishRaw
   :project: helics

.. doxygenfunction:: helicsPublicationPublishString
   :project: helics

.. doxygenfunction:: helicsPublicationPublishVector
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

.. doxygenfunction:: helicsSubscriptionGetComplex
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetDouble
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetInteger
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetKey
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetRawValue
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetString
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetType
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetUnits
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetValueSize
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetVector
   :project: helics

.. doxygenfunction:: helicsSubscriptionGetVectorSize
   :project: helics

.. doxygenfunction:: helicsSubscriptionIsUpdated
   :project: helics

.. doxygenfunction:: helicsSubscriptionLastUpdateTime
   :project: helics

.. doxygenfunction:: helicsSubscriptionSetDefaultComplex
   :project: helics

.. doxygenfunction:: helicsSubscriptionSetDefaultDouble
   :project: helics

.. doxygenfunction:: helicsSubscriptionSetDefaultInteger
   :project: helics

.. doxygenfunction:: helicsSubscriptionSetDefaultRaw
   :project: helics

.. doxygenfunction:: helicsSubscriptionSetDefaultString
   :project: helics

.. doxygenfunction:: helicsSubscriptionSetDefaultVector
   :project: helics

.. doxygenenumvalue:: helics_clone_filter
   :project: helics

.. doxygenenumvalue:: helics_custom_filter
   :project: helics

.. doxygenenumvalue:: helics_delay_filter
   :project: helics

.. doxygenenumvalue:: helics_discard
   :project: helics

.. doxygenenumvalue:: helics_error
   :project: helics

.. doxygenenumvalue:: helics_error_state
   :project: helics

.. doxygenenumvalue:: helics_execution_state
   :project: helics

.. doxygenvariable:: helics_false
   :project: helics

.. doxygenenumvalue:: helics_finalize_state
   :project: helics

.. doxygenenumvalue:: helics_initialization_state
   :project: helics

.. doxygenenumvalue:: helics_invalid_argument
   :project: helics

.. doxygenenumvalue:: helics_invalid_function_call
   :project: helics

.. doxygenenumvalue:: helics_invalid_object
   :project: helics

.. doxygenenumvalue:: helics_invalid_state_transition
   :project: helics

.. doxygenenumvalue:: helics_ok
   :project: helics

.. doxygenenumvalue:: helics_pending_exec_state
   :project: helics

.. doxygenenumvalue:: helics_pending_init_state
   :project: helics

.. doxygenenumvalue:: helics_pending_iterative_time_state
   :project: helics

.. doxygenenumvalue:: helics_pending_time_state
   :project: helics

.. doxygenenumvalue:: helics_randomDelay_filter
   :project: helics

.. doxygenenumvalue:: helics_randomDrop_filter
   :project: helics

.. doxygenenumvalue:: helics_reroute_filter
   :project: helics

.. doxygenenumvalue:: helics_startup_state
   :project: helics

.. doxygenenumvalue:: helics_terminated
   :project: helics

.. doxygenvariable:: helics_time_epsilon
   :project: helics

.. doxygenvariable:: helics_time_zero
   :project: helics

.. doxygenvariable:: helics_true
   :project: helics

.. doxygenenumvalue:: helics_warning
   :project: helics

.. doxygenenumvalue:: iterate_if_needed
   :project: helics

.. doxygenenumvalue:: iterating
   :project: helics

.. doxygenenumvalue:: iteration_error
   :project: helics

.. doxygenenumvalue:: iteration_halted
   :project: helics

.. doxygenstruct:: message_t
   :project: helics

.. doxygenenumvalue:: next_step
   :project: helics

.. doxygenenumvalue:: no_iteration
   :project: helics



```
