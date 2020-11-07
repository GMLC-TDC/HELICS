# Federate flags

There are a number of flags which control how a federate acts with respect to timing and its interfaces. The Timing flags and controls are described [here](./Timing.html). There are also a number of other flags which control some aspects of the interfaces, and a few other flags which can be applied to specific interfaces.

## single_thread_federate

If specified in the federateInfo on creation this tells the core that this federate will only execute in a single thread and only a single federate is interacting with the connected core.

NOTE: This option is not fully enabled and won't be fully available until HELICS 3.0 is released.

This disables the asynchronous functions in the federate and turns off a number of protection mechanisms for handling federate interaction across multiple threads. This can be used for performance reasons and can interact with the single_thread core types that are in development.

## ignore_time_mismatch_warnings

If certain timing options are used this can cause the granted time to be greater than the requested time. For example with the `period`, or `minTimeDelta` specified. This situation would normally generate a warning message, but if this option is enabled those warnings are silenced.

## connections_required

When an interface requests a target it tries to find a match in the federation. If it cannot find a match at the time the federation is initialized, then the default is to generate a warning. This will not halt the federation but will display a log message. If the connections required flag is set on a federate all subsequent `addTarget` calls on any interface will generate an error if the target is not available. If the addTarget is made after the initialization point, the error is immediate.

## connections_optional

When an interface requests a target it tries to find a match in the federation. If it cannot find a match at the time the federation is initialized, then the default is to generate a warning. This will not halt the federation but will display a log message. If the `connections_optional` flag is set on a federate all subsequent `addTarget` calls on any interface will not generate any message if the target is not available.

## strict_input_type_checking

This applies to Input interface. If enabled this flag tells the inputs to check that the type matches.

## slow_responding

If specified on a federate it indicates the federate may be slow in responding, and to not disconnect the federate if things are slow.
If applied to a core or broker, it is indicative that the broker doesn't respond to internal pings quickly so they cannot be used as a mechanism for timeout. For federates this option doesn't do much but its role will likely be expanded as more robust timeout and coordination mechanics are developed.

## debugging

If a program is being debugged and may halt execution the `--debugging` flag may be used to turn off some timeouts and keep everything working a little more smoothly. This flag is the equivalent of "--slow_responding" for a federate and "--slow_responding --disable_timer` for a broker/core.

## terminate on error

If the `terminate_on_error` flag is set then a federate encountering an internal error will trigger a global error and cause the entire federation to abort. If the flag is not set then errors will only be local. Errors of this nature are typically the result of configuration errors. For example having a required publication that is not used or incompatible units or types on publications and subscriptions.
