# Configuration Options Reference

```eval_rst
.. toctree::
    :hidden:
    :maxdepth: 2
```
xxxxxxx Update Julia links to specific points in page

xxxxxxx Add horizontal rule in each section

Many of the HELICS entities have significant configuration options. Rather than comprehensively list these options while explaining the features themselves, we've created this section of the User Guide to serve as a reference as to what they are, what they do, and how to use them. This reference is oriented-around the use of JSONs for configuration and is an attempt to be comprehensive in listing and explaining those options. As will be explained below, many of these options are accessible via direct API calls though some of these calls are general in nature (such as [helicsFederateInfoSetIntegrerProperty](https://docs.helics.org/en/v3userguide/c-api-reference/index.html) to set the logging level, among other things). As such

## Configuration methods

Generally, there are three ways in which a co-simulation can be configured and all the various options can be defined:

1. Using direct API calls in the federate source code.
2. Using command-line switches/flags which beginning execution of the federate
3. Using a JSON configuration file (and calling helicsCreateValueFederateFromConfig, helicsCreateMessageFederateFromConfig, or helicsCreateComboFederateFromConfig)

Not all configuration options are available in all three forms but often they are. For example, it is not possible (nor convenient) to configure a publication for a federate from the command line but it is possible to do so with the JSON config file and with API calls. 

### Choosing configuration method
Which method you use to configure your federate and co-simulation significantly depends on the circumstances of the co-simulation and details of any existing code-base being used. Here is some guidance, though, to help in guiding you're decision in how to do this:

* **If possible, use a JSON configuration file** - Using a JSON configuration file allows creates separation between the code base of the federation and its use in a particular co-simulation. This allows for a modularity between the functionality the federate provides and the particular co-simulation in which it is applied.  For example, a power system federate can easily be reconfigured to work on one model vs another through the use of a JSON configuration file. The particular publications and subscriptions may change but the main functionality of the federate (solving the power flow) does not.
* **JSON configuration produces a natural artifact that defines the co-simulation** - Another advantage of the external configuration in the JSON file is that it is a human-readable artifact that can be distributed separately from the source code that provides a lot of information about how the co-simulation was run. In fact, its possible to just look at the configuration files of a federation and do some high-level debugging (checking to see that the subscriptions and publications are aligned, for example).
* **New federates in ill-defined use cases may benefit from API configuration** - The modularity that the JSON config provides may not offer many benefits if the federate is newly integrated into HELICS and/or is part of an evolving analysis. During these times the person(s) doing the integration may just want to make direct API calls instead of having to mess with writing the federate code and a configuration file. There will likely be a point in the future when the software is more codified and switching to a JSON configuration makes more sense. 
* **Command-line configuration (where possible) allows for small, quick changes to the configuration** - Because the command line doesn't provide comprehensive access to the necessary configuration, it will never be a stand-alone configuration option but it does have the advantage of providing quick access right as a user is instantiating the federate. This is an ideal place to make small changes to the configuration (e.g. changing the minimum time step) without having to edit any files. 
* **API configuration is most useful for dynamic configuration** - If there is a need to change the configuration of a given federate dynamically, the API is the only way to do that. Such needs are not common but there are cases where, for example, it may be necessary to define the configuration based on the participants in the federation (_e.g._ publications, subscriptions, timing). It's possible to use [queries](./user_guide/queries.md) to understand the composition and configuration of the federation and then use the APIs to define the configuration of the federate in question. 

### Precedence
xxxxxxx - What happens if an option is defined on the JSON config, command line, and with a specific API call? Who wins?

### How to Use This Reference
The remainder of this reference lists the configuration options that are supported in the JSON configuration file. Where possible, the corresponding C++ API calls and the links to that documentation will be provided. Generally, the command-line options use the exact same syntax as the JSON configuration options preceded by a `--` and followed by either an `=` or a space and then the parameter value (_i.e._ `--name testname`). In the cases where a single letter switch is available, that will be listed (_i.e._ `-n testname`).

Default values are shown in "[]" following the name(s) of the option.

xxxxxxx is this the place to talk about how the JSON config is invoked? 

## Sample Configurations
The JSON configuration file below shows all the configuration options in a single file along with their default values and shows what section of the file they should be placed in. Most JSON configuration files will require far fewer options than shown here; items marked with "**" are required. Many items have alternative names that are 

Many of the configuration parameters have alternate names that provide the same functionality. These sets of names are shown in the example below with each one quoted and separated by a "|".

An example of one publication, subscription, named input, endpoint, and filter is also shown. The values for each of these options is arbitrary and in the case of filters, many more values are supported and a description of each is provided.

(Note that JSON does not support comments and thus the block below is not valid JSON.)

```json
{
  // General
  **"name": "arbitrary federate name",**
  "coreType" | "coretype": "zmq",
  "corename": "core name",
  "coreinit" | "coreinitstring" : "",
  "autobroker": false,
  "connections_required": false,
  "connections_optional": false,
  "strict_input_type_checking": false,
  "terminate_on_error" | "terminateOnError" | "terminateonerror": false,
  "source_only" | "sourceonly" | "sourceOnly" | "source": false,
  "observer": false,
  "only_update_on_change": false,
  "only_transmit_on_change": false,
  
  //Logging
  "logfile": "output.log"
  "log_level" | "loglevel" | "logLevel": 5,
  "force_logging_flush": false,
  "file_log_level": "",
  "console_log_level": "",
  "dumplog": false,
  
  //Timing
  "ignore_time_mismatch_warnings": false,
  "uninterruptible" | "ignore_interrupts" | "ignoreinterrupts" | "nointerrupts" | "no_interrupts": false,
  "period": 0,
  "offset": 0,
  "time_delta" | "timedelta" | "timeDelta" | "delta": 0,
  "minTimeDelta": 0,
  "input_delay" | "inputdelay" | "inputDelay": 0,
  "output_delay" | "outputdelay" | "outputDelay": 0,
  "real_time" | "realTime": false,
  "rt_tolerance" | "rttolerance" | "rtTolerance": 0.2,
  "rt_lag" | "rtlag" | "rtLag": 0.2,
  "rt_lead" | "rtlead" | "rtLead": 0.2,
  "wait_for_current_time_update" | "wait_for_current_time" | "waitforcurrenttimeupdate" | "waitforcurrenttime": false,
  "restrictive_time_policy" | "conservative_time_policy" | "restrictive_time" | "conservative_time" | "restrictiveTime" | "conservativeTime": false,
  "slow_responding" | "slow_response" | "slowResponding" | "slow" | "no_ping" | "disable_ping": false,
  "forward_compute": false,
  "delayed_update" | "delayedUpdate": false,
  
  

  //Iteration
  "rollback": false,
  "max_iterations" | "maxiterations" | "maxIteration" | "iterations": 10,
  
  
  //Network
  "interfaceNetwork": "local",
  "brokeraddress": xxxxxxx, default?
  "reuse_address": false,
  "noack": false,
  "maxsize": xxxxxxx, default?
  "maxcount": xxxxxxx, default?
  "networkretries": xxxxxxx, default?
  "osport": xxxxxxxx, default?
  "brokerinit": "",
  "server_mode": "client",
  "interface": (local IP address),
  "port": xxxxxxx, default? 
  "brokerport": xxxxxxx, default?
  "localport": xxxxxxx, default?
  "portstart": xxxxxxx, default?
   
  
  "publications" | "subscriptions" | "inputs": [
    {
      **"key": "publication key",**
      "type": xxxxxxx, Required or has a default value?
      "unit": "m",
      "global": false,
      "optional" | "connectionoptional" | "connection_optional": true, 
      "required" | "connectionrequired" | "connection_required": false,
      "tolerance": -1,
      "targets": xxxxxxx, (I'm not sure I'm reading the code correctly; this may not be a flag)
      "buffer_data" | "bufferdata" | "bufferData":  false, indication the publication should buffer data
      "strict_type_matching" | "strict_type_checking" | "strict_input_type_matching" | "strict_input_type_checking" | "strictinputtypechecking" | "strictinputtypematching" | "stricttypechecking" | "stricttypematching" | "strict": false, Requires data types on pubs and corresponding subs to match
      "shortcut" | "alias": xxxxxxx, (What does this mean?)
      "ignore_unit_mismatch" | "ignore_units": false,
      "info": xxxxxxx (supported in pubs?)
    },
  ],
  "publications" :[
  	{
  		"only_transmit_on_change" | "onlytransmitonchange": false,
  	}
  ]	,
  "subscriptions": [
    {
      "only_update_on_change" | 'onlyupdateonchange": false,
    }
  ],
  "inputs": [
    {
      "connections": 1,
      "input_priority" | "priority" | "input_priority_location" | "priority_location": 0,
      "clear_priority_list" | "clear_priority": possible to have this as a config option?
      "single_connection_only" | "single" | "single_connection" | "singleconnection" | "singleconnectionsonly": false,
      "multiple_connections_allowed" | "multiple" | "multiple_connections" | "multipleconnections" | "multipleconnectionsallowed" : false
      "multi_input_handling_method" | "multi_input_handling": "average",
      "targets": ["pub1", "pub2"]
    }
  ],
  "endpoints": [
    {
      "name": "endpoint name",
      "type": "endpoint type",
      "global": true,
      "destination" | "target" : "default endpoint destination",
      "shortcut": xxxxxxx, Supported?
      "subscriptions": "",
      "filters": "",
      "info": ""
    }
  ],
  "filters": [
    {
      "name": "filter name",
      "targets" | "sourcetargets" | "sourceTargets": "endpoint name",
      "desttargets" | "destTargets": "endpoint name",
      "info": "",
      "operation": "randomdelay",
      "properties": {
        "name": "delay",
        "value": 600
      }
    }
  ] 
}



  

  
 
  
```

## General Federate Options

There are a number of flags which control how a federate acts with respect to timing and its signal interfaces.

### `name` | `-n` (required)
_API:_ `helicsFederateInfoSetCoreName` 
[C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a1fc4b4563bd06ac54d9569d1df5f8d0c)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

Every federate must have a unique name across the entire federation; this is functionally the address of the federate and is used to determine where HELICS messages are sent. An error will be generated if the federate name is not unique.


### `core_type` | `coretype` | `coreType` | `-t` ["zmq"]
_API:_ `helicsFederateInfoSetCoreTypeFromString`
[C++](https://docs.helics.org/en/latest/doxygen/classhelicscpp_1_1FederateInfo.html#a94654cba67de8d4aaf47cd99bbbd5d60)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

There are a number of technologies or message buses that can be used to send HELICS messages among federates. Every HELICS enabled simulator has code in it that creates a core which connects to a HELICS broker using one of these messaging technologies. ZeroMQ (zmq) is the default core type and most commonly used but there are also cores that use TCP and UDP networking protocols directly (forgoing ZMQ's guarantee of delivery and reconnection functions), IPC (uses Boost's interprocess communication for fast in-memory message-passing but only works if all federates are running on the same physical computer), and MPI (for use on HPC clusters where MPI is installed). (xxxxxxxx add link to core type pages)



### `core_name` | `corename` | `coreName` []
_command line:_ xxxxxxx

_API:_ xxxxxxxx

Only applicable for `ipc` and `test` core types; otherwise can be left undefined.



### `core_init_string` | `coreinitstring` | `coreInitString` | `-i` []
_API:_ `helicsFederateInfoSetCoreInitString`
[C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a472ea0a8ff1a57d91bfa01b04137e2a8)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

A command-line-like string that specifies options for the core as it connects to the federation. These options are:

- `--broker=` | `broker_address=` | `brokeraddress`: IP address of broker
- `--brokerport=`: Port number on which the broker is communicating
- `--broker_rank=`: For MPI cores only; identifies the MPI rank of the broker
- `--broker_tag=`: For MPI cores only; identifies the MPI tag of the broker
- `--localport=`: Port number to use when communicating with this core
- `--autobroker`: When included the core will automatically generate a broker
- `--key=`: Specifies a key to use when communicating with the broker. Only federates with this key specified will be able to talk to the broker with the same `key` value. This is used to prevent federations running on the same hardware from accidentally interfering with each other.

xxxxxxxx Figure out how to represent all the options shown in BrokerBase.cpp




### `autobroker` [false]
_API:_ (none)

Automatically generate a broker if one cannot be connected to. xxxxxxxx Need more information.


### `broker_init_string` | `brokerinitstring` | `brokerInitString` [""]
_API:_ (none)

String used to define the configuration of the broker if one is autogenerated. xxxxxxxx Need more information.



### `terminate_on_error` | `terminateonerror` | `terminateOnError` [false]
_API:_ (none)

If the `terminate_on_error` flag is set then a federate encountering an internal error will trigger a global error and cause the entire federation to terminate. Errors of this nature are typically the result of configuration errors, such as having a required publication that is not used or incompatible units or types on publications and subscriptions.



### `source_only` | `sourceonly` | `sourceOnly` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_source_only`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

Used to indicate to the federation that this federate is only producing data and has no inputs/subscriptions. Specifying this when appropriate allows HELICS to more efficiently grant times to the federation.



### `observer` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_observer`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

Used to indicate to the federation that this federate produces no data and only has inputs/subscriptions. Specifying this when appropriate allows HELICS to more efficiently grant times to the federation.



## Logging Options

### `log_file` | `logfile` | `logFile` []
_API:_ `helicsFederateSetLogFile`
[C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#abde89169985b8a18c2d1b8fa803e5169)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federate)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

Specifies the name of the log file where logging messages will be written.



### `log_level` | `loglevel` | `logLevel` [0]
_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#ad6a898deb8df83ee31d62eccbb202aef)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_int_log_level`

_Valid values:_ 

- -1 - no logging - `helics_log_level_no_print`
- 0 - error - `helics_log_level_error`
- 1 - warning - `helics_log_level_warning`
- 2 - summary - `helics_log_level_summary`
- 3 - connections - `helics_log_level_connections`
- 4 - interfaces - `helics_log_level_interfaces`
- 5 - timing - `helics_log_level_timing`
- 6 - data - `helics_log_level_data`
- 7 - trace - `helics_log_level_trace`

Determines the level of detail for log messages. All messages at the user-provided level and lower will be printed to the log file. Valid levels and their corresponding enumerations are shown below.






### `file_log_level` | `fileloglevel` | `fileLogLevel` [null]
_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#ad6a898deb8df83ee31d62eccbb202aef)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_int_file_log_level`

_Valid values:_ Same as in `loglevel`

Allows a distinct log level for the written log file to be specified. By default the logging level to file and console are identical and will only differ if `file_log_level` or `console_log_level` are defined.



### `console_log_level` | `consoleloglevel` | `consoleLogLevel` [null]
_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#ad6a898deb8df83ee31d62eccbb202aef)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_int_console_log_level`

_Valid values:_ Same as in `loglevel`

Allows a distinct log level for the written log file to be specified. By default the logging level to file and console are identical and will only differ if `file_log_level` or `console_log_level` are defined.




### `force_logging_flush` | `forceloggingflush` | `forceLoggingFlush` [false]
_API:_ (none)

Setting this option forces HELICS logging messages to be flushed to file after each one is written. This prevents the buffered IO most OSs implement to be bypassed such that all messages appear in the log file immediately after being written at the cost of slower simulation times due to more time spent writing to file.


### `dump_log` | `dumplog` | `dumpLog` [false]
_API:_ (none)

When set, a record of all messages is captured and written out to the log file at the conclusion of the co-simulation.



## Timing Options

### `ignore_time_mismatch` | `ignoretimemismatch` | `ignoreTimeMismatch` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_ignore_time_mismatch_warnings`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

If certain timing options (_i.e._ `period`, or `minTimeDelta`) are used it is possible for the time granted a federate to be greater than the requested time. This situation would normally generate a warning message, but if this flag is set those warnings are silenced. 




### `uninterruptible` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_uninterruptible`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

Normally, a federate will be granted a time earlier than it requested when it receives a message from another federate; the presence of any message implies there could be an action the federate needs to take and may generate new messages of its own. There are times, though, when it is important that the federate only be granted a time (and begin simulating/executing again) that it has previously requested. For example, there could be some controller that should only operate at fixed intervals even if new data arrives earlier. In these cases, setting the `uninterruptible` flag will prevent premature time grants.




### `period` [0]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_time_period`

_Valid values:_ 

* **C/Python/Julia/Nim**: value of type `helics_time` 
* **C++**: value of type `Time`

Many time-based simulators have a minimum time-resolution or a user-configurable step size. The `period` parameter can be used to effectively synchronize the times that are granted with the defined simulation period. The default units for `period` are in seconds but the string for this parameter can include its own units (e.g. "2 ms" or "1 hour"). Setting `period` will force all time grants to occur at times of `n*period` even if subscriptions are updated, messages arrive, or the federate requests a time between periods. This value effectively makes the federates `uninterruptible` during the times between periods. Relatedly...



### `offset` [0]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_time_offset`

_Valid values:_ 

* **C/Python/Julia/Nim**: value of type `helics_time` 
* **C++**: value of type `Time`

There may be cases where it is preferable to have a simulator receive time grants that are offset slightly in time to one or more other federates. Defining an `offset` value allows this to take place; units are handled the same as in `period`. Setting both `period` and `offset`, will result in the all times granted to the federate in question being constrained to `n*period + offset`.



### `time_delta` | `timeDelta` | `timedelta` [1ns]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_time_delta`

_Valid values:_ 

* **C/Python/Julia/Nim**: value of type `helics_time` 
* **C++**: value of type `Time`

timeDelta has some similarities to `period`; where `period` constrained the granted time to regular intervals, `timeDelta` constrains the grant time to a minimum amount from the last granted time. Units are handled the same as in `period`.




### `input_delay` | `inputdelay` | `inputDelay` [0]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_time_input_delay`

_Valid values:_ 

* **C/Python/Julia/Nim**: value of type `helics_time` 
* **C++**: value of type `Time`

`inputDelay` specifies a delay in simulated time between when a signal arrives at a federate and when that federate is notified that a new value is available. `outputDelay` is similar but applies to signals being sent by a federate. Note that this applies to both value signals and message signals. (xxxxxxx - Need to clarify with developers if messages are delayed t_filter + t_outputDelay or max(t_filter, t_outputDelay)



### `output_delay` | `outputdelay` | `outputDelay` [0]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_time_output_delay`

_Valid values:_ 

* **C/Python/Julia/Nim**: value of type `helics_time` 
* **C++**: value of type `Time`

`outputDelay` is similar to `input_delay` but applies to signals being sent by a federate. Note that this applies to both value signals and message signals. (xxxxxxx - Need to clarify with developers if messages are delayed t_filter + t_outputDelay or max(t_filter, t_outputDelay)




### `real_time` | `realtime` | `realTime` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_realtime`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

If set to true the federate uses `rt_lag` and `rt_lead` to match the time grants of a federate to the computer wall clock.
If the federate is running faster than real time this will insert additional delays. If the federate is running slower than real time this will cause a force grant, which can lead to non-deterministic behavior. `rt_lag` can be set to maxVal to disable force grant


### `rt_lag`| `rtlag` | `rtLag` [0.2] and `rt_lead` | `rtlead` | `rtLead` [0.2]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_time_rt_lag` and `helics_property_time_rt_lead`

_Valid values:_ 

* **C/Python/Julia/Nim**: value of type `helics_time` 
* **C++**: value of type `Time`

Defines "real-time" for HELICS by setting tolerances for HELICS to use when running a real-time co-simulation. HELICS is forced to keep simulated time within this window of wall-clock time. Most general purpose OSes do not provide guarantees of execution timing and thus very small values of `rt_lag` and `rt_lead` (less than 0.005) are not likely to be achievable.



### `rt_tolerance` | `rttolerance` | `rtTolerance` [0.2]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_time_rt_tolerance`

_Valid values:_ 

* **C/Python/Julia/Nim**: value of type `helics_time` 
* **C++**: value of type `Time`

Implements the same functionality of `rt_lag` and `rt_lead` but does so by using a single value to set symmetrical lead and lag constraints. 



### `wait_for_current_time_update` |`waitforcurrenttimeupdate` | `waitForCurrentTimeUpdate` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_wait_for_current_time_update`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

If set to true a federate will not be granted the requested time until all other federates have completed at least 1 iteration of the current time or have moved past it. If it is known that 1 federate depends on others in a non-cyclic fashion, this can be used to optimize the order of execution without iterating.




### `restrictive_time_policy` | `restrictivetimepolicy` | `restrictiveTimePolicy` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_restrictive_time_policy`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

If set, a federate will not be granted the requested time until all other federates have completed at least 1 iteration of the current time or have moved past it. If it is known that 1 federate depends on others in a non-cyclic fashion, this can be used to optimize the order of execution without iterating.

Using the option `restrictive-time-policy` forces HELICS to use a fully conservative mode in granting time. This can be useful in situations beyond the current reach of the distributed time algorithms. It is generally used in cases where it is known that some federate is executing and will trigger someone else, but most federates won't know who that might be. This prevents extra messages from being sent and a potential for time skips. It is not needed if some federates are periodic and execute every time step. The flag can be used for federates, brokers, and cores to force very conservative timing with the potential loss of performance as well.

Only applicable to Named Input interfaces ([see section on value federate interface types](./user-guide/value_federates.md)), if enabled this flag checks that data type of the incoming signals match that specified for the input. (xxxxxxx - What happens if they don't match?)




### `slow_responding` | `slowresponsing` | `slowResponding` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_slow_responding`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

If specified on a federate, setting this flag indicates the federate may be slow in responding, and to not forcibly eject the federate from the federation for the slow response. This is an uncommon scenario.

If applied to a core or broker (xxxxxxx need examples of this syntax), it is indicative that the broker doesn't respond to internal pings quickly and should not be disconnected from the federation for the slow response.











## Iteration

### `forward_compute` | `forwardcompute` | `forwardCompute` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_slow_responding`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

Indicates to the broker and the rest of the federation that this federate computes ahead of its granted time and can/does roll back when necessary. xxxxxxx - How does this affect co-simulation operation?




### `rollback` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_slow_responding`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

Indicates to the broker and the rest of the federation that this federate can/does roll back when necessary. xxxxxxx - How does this affect co-simulation operation?



### `max_iterations` | `maxiterations` | `maxIteration` [50]
_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#ad6a898deb8df83ee31d62eccbb202aef)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_property_int_max_iterations`

_Valid values:_ integer value

For federates engaged in iteration (recomputing values based on updated inputs at a single simulation timestep) there may be a need to enforce a maximum number of iterations. This option allows that value to be set. xxxxxxx What triggers this limit (number of times the same time is requested?)? What happens when the limit is reached?


## Network

xxxxxxxx Most of the information missing here is context. The options are more or less self-explanatory but the context in which you might need to use them is not clear. I'm assuming many of these were motivated by NAERM and the AWS environment.



### `interface_network` | `interfacenetwork` | `interfaceNetwork` [local interface]



### `reuse_address` | `reuseaddress` | `reuseAddress` [false]
_API:_ (none)

allow the server to reuse a bound address, mostly useful for tcp cores. xxxxxxx Need more information here.



### `noack_connect` | `noackconnect` | `noackConnect` [false]
_API:_ `noAckConnection`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1NetworkBrokerData.html#a925cbe8ddd7e612687184fb029b59822)

specify that a connection_ack message is not required to be connected with a broker. xxxxxxx Need more information here.



### `max_size` | `maxsize` | `maxSize`[4096]
_API:_ (none)

Message buffer size. xxxxxxx Need more information here.



### `max_count` | `maxcount` | `maxCount` [256]
_API:_ (none)

Maximum number of messages in queue. xxxxxxx Need more information here.



### `network_retries` | `networkretries` | `networkRetries` [5]
_API:_ (none)
Maximum number of network retry attempts. xxxxxxx Need more information here.


### `use_os_port` | `useosport` | `useOsPort` [false]
_API:_ (none)
Setting this flag specifies that the OS should set the port for the HELICS message bus. xxxxxxx Need more information here.



### `client` or `server` [null]
_API:_ (none)
specify that the network connection should be a server or client. By default neither option is enabled. xxxxxxx Need more information here. Are these just flags?


### `local_interface` | `localinterface` | `localInterface` [local address]
_API:_ (none)
the local interface to use for the receive ports. xxxxxxx Need more information here.



### `port` | `-p` []
_API:_ (none)
Port number to use. xxxxxxx Need more information here. xxxxxxx Need more information here.



### `broker_port` | `brokerport` | `brokerPort` []
_API:_ (none)

The port to use to connect to the broker. xxxxxxx Need more information here.



### `local_port` | `localport` | `localPort` []
_API:_ (none)
port number for the local receive port. xxxxxxx Need more information here.



### `port_start` | `portstart` | `portStart` []
_API:_ (none)
starting port for automatic port definitions. xxxxxxx Need more information here.








## General and Per Subscription, Input, or Publication
These options can be set globally for all subscriptions, inputs and publications for a given federate. Even after setting them globally, they can be included in the configuration for an individual subscription, input, or publication, over-riding the global setting.



### `only_update_on_change` | `onlyupdateonchange` | `onlyUpdateOnChange` [false] and `only_transmit_on_change` | `onlytransmitonchange` | `onlyTransmitOnChange` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `helics_flag_only_update_on_change` and `helics_flag_only_transmit_on_change`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

Setting these flags prevents new value signals with the same value from being received by the federate or sent by the federate. Setting these flags will reduce the amount of traffic on the HELICS bus and can provide performance improvements in co-simulations with large numbers of messages.


### `tolerance`
_API:_ `helicsPublicationSetMinimumChange` and `helicsInputSetMinimumChange`
[C++ input](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Input.html#a55056ac9dd2895270f575827dd9951c7) and [C++ publication](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Publication.html#ab66f5680bb4a5e062314f6f8e5dea846)
[C/Python input](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo) and [C/Python publication](https://docs.helics.org/en/latest/c-api-reference/index.html#publication)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

This option allows the specific numerical definition of "change" when using the `only_update_on_change` and `only_transmit_on_change` options.




### `connection_required` | `connectionrequired` | `connectionRequired` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `required_flag`

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`



When a federate is initialized, one of its tasks is to make sure the recipients of directed signals exist. If, after the federation is initialized, the recipient can't be found, then by default a warning is generated and written to the log file. If the `connections_required` flag is set, this warning becomes a fatal error that stops the co-simulation. 

- `publications` - At least one federate must subscribe to the publications.
- `subscriptions` - The message being subscribed to must be provided by some other publisher in the federation.


### `connection_optional` | `connectionoptional` | `connectioOptional` [false]
_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
[C/Python](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
[Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/#Functions)

_Property's enumerated name:_ `optional_flag `

_Valid values:_ 

* **C/Python/Julia/Nim**: `helics_true` or `helics_false`
* **C++**: `true` or `false`

When an interface requests a target it tries to find a match in the federation. If it cannot find a match at the time the federation is initialized, then the default is to generate a warning. This will not halt the federation but will display a log message. If the `connections_optional` flag is set on a federate all subsequent `addTarget` calls on any interface will not generate any message if the target is not available.









## Subscription, Input, and/or Publication Options
These options are valid for subscriptions, inputs, and/or publications.

### `key` (required)
- `publications` - The string in this field is the unique identifier (at the federate level) for the value that will be published to the federation. If `global` is set (see below) it must be unique to the entire federation.
- `subscriptions` - This string identifies the federation-unique value that this federate wishes to receive. Unless `global` has been set to `true` in the publishings JSON configuration file, the name of the value is formatted as `<federate name>/<publication key>`. Both of these strings can be found in the publishing federate's JSON configuration file as the `name` and `key` strings, respectively. If `global` is `true` the string is just the `key` value.
- `input` - xxxxxxx

### `type` [null]
HELICS supports data types and data type conversion ([as best it can](https://www.youtube.com/watch?v=mZOAn-3aATY)).


### `unit` [null]
HELICS is able to do some levels of unit conversion, currently only on double type publications but more may be added in the future. The units can be any sort of unit string, a wide assortment is supported and can be compound units such as m/s^2 and the conversion will convert as long as things are convertible. The unit match is also checked for other types and an error if mismatching units are detected. A warning is also generated if the units are not understood and not matching. The unit checking and conversion is only active if both the publication and subscription specify units.

### `global` [false]
(publications only) `global` is used to indicate that the value in `key` will be used as a global name when other federates are subscribing to the message. This requires that the user ensure that the name is used only once across all federates. Setting `global` to `true` is handy for federations with a small number of federates and a small number of message exchanges as it allows the `key` string to be short and simple. For larger federations, it is likely to be easier to set the flag to `false` and accept the extra naming


### `targets` []
For 

### `buffer_data` | `bufferdata` | `bufferData` [false]
indicator that the publication should buffer data. xxxxxxxx

### `shortcut` [null]

### `info` [""]

The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example.




## Input-only Options

### `connections` []

### `input_priority_location` | `inputprioritylocation` | `inputPriorityLocation` []

### `clear_priority_list` | `clearprioritylist` | `clearPriorityList` [false]

### `single_connection_only` | `singleconnectiononly` |`singleConnectionOnly` [false]

### `multiple_connections_allowed` | `multipleconnectionsallowed` | `multipleConnectionsAllowed` [true]

### `multi_input_handling_method` | `multiinputhandlingmethod` | `multiInputHandlingMethod` [`none`]
- `none` | `no_op`
- `or`
- `sum`
- `max`
- `min`
- `average`
- `mean`
- `vectorize`
- `diff`

### `targets` []


## Endpoint Options

### `name` (required)

### `type` []

### `destination` | `target` []

### `shortcut` []

### `subscriptions` []

### `filters` [null]

### `info` [""]



## Filter Options

Filters are registered with the core or through the application API.
There are also Filter object that hide some of the API calls in a slightly nicer interface. Generally a filter will define a target endpoint as either a source filter or destination filter. Source filters can be chained, as in there can be more than one of them. At present there can only be a single non-cloning destination filter attached to an endpoint.

Non-cloning filters can modify the message in some way, cloning filters just copy the message and may send it to multiple destinations.

On creation, filters have a target endpoint and an optional name.
Custom filters may have input and output types associated with them.
This is used for chaining and automatic ordering of filters.
Filters do not have to be defined on the same core as the endpoint, and in fact can be anywhere in the federation, any messages will be automatically routed appropriately.

### `name` (required)

### `source_targets`, `sourcetargets`, `sourceTargets` []

### `destination_targets`, `destinationtargets`, `destinationtargets` []

### `info` [""]

### `operation` []
Filters have a predefined set of operations they can perform.  The following list defines the valid operations for filters. Most filters require additional specifications in properties data structure, an example of which is shown for each filter type.


##### `reroute`

This filter reroutes a message to a new destination. it also has an optional filtering mechanism that will only reroute if some patterns are matching. The patterns should be specified by "condition" in the set string the conditions are regular expression pattern matching strings.

Example `property` object:

```json
...
   "operation": "reroute",
	"properties": [
		{
			"name": "newdestination",
			"value": "endpoint name	"
		},
		{
			"name": "condition",
			"value": "regular expression string"
		}
	]
...
```


##### `delay` 

This filter will delay a message by a certain amount fo time. 

Example `property` object:

```json
...
   "operation": "delay",
	"properties": {
		"name": "delay", 
		"value": "76 ms", 
	},
...
```

#####  `random_delay` | `randomdelay` | `randomDelay`

This filter will randomly delay a message according to specified random distribution
available options include distribution selection, and 2 parameters for the distribution
some distributions only take one parameter in which case the second is ignored. The distributions available are based on those available in the C++ [random](http://www.cplusplus.com/reference/random/) library. 

- **constant** 
	- param1="value" this just generates a constant value
- [**uniform**](http://www.cplusplus.com/reference/random/uniform_real_distribution/)
  - param1="min"
  - param2="max"
- [**bernoulli**](http://www.cplusplus.com/reference/random/bernoulli_distribution/) - the bernoulli distribution will return param2 if the bernoulli trial returns true, 0.0 otherwise. Param1 is the probability of returning param2
  - param1="prob"
  - param2="value"
- [**binomial**](http://www.cplusplus.com/reference/random/binomial_distribution/)
  - param1=t (cast to int)
  - param2="p"
- [**geometric**](http://www.cplusplus.com/reference/random/geometric_distribution/)
  - param 1="prob" the output is param2\*geom(param1) so multiplies the integer output of the geometric distribution by param2 to get discrete chunks
- [**poisson**](http://www.cplusplus.com/reference/random/poisson_distribution/)
  - param1="mean"
- [**exponential**](http://www.cplusplus.com/reference/random/exponential_distribution/)
  - param1="lambda"
- [**gamma**](http://www.cplusplus.com/reference/random/gamma_distribution/)
  - param1="alpha"
  - param2="beta"
- [**weibull**](http://www.cplusplus.com/reference/random/weibull_distribution/)
  - param1="a"
  - param2="b"
- [**extreme_value**](http://www.cplusplus.com/reference/random/extreme_value_distribution/)
  - param1="a"
  - param2="b"
- [**normal**](http://www.cplusplus.com/reference/random/normal_distribution/)
  - param1="mean"
  - param2="stddev"
- [**lognormal**](http://www.cplusplus.com/reference/random/lognormal_distribution/)
  - param1="mean"
  - param2="stddev"
- [**chi_squared**](http://www.cplusplus.com/reference/random/chi_squared_distribution/)
  - param1="n"
- [**cauchy**](http://www.cplusplus.com/reference/random/cauchy_distribution/)
  - param1="a"
  - param2="b"
- [**fisher_f**](http://www.cplusplus.com/reference/random/fisher_f_distribution/)
  - param1="m"
  - param2="n"
- [**student_t**](http://www.cplusplus.com/reference/random/student_t_distribution/)
  - param1="n"

```json
...
   "operation": "randomdelay",
	"properties": [
		{
			"name": "distribution",
			"value": normal
		},
		{
			"name": "mean",
			"value": 0
		},
		{
			"name": "stdev",
			"value": 1
		}
	],
...
```


##### `random_drop` | `randomdrop` | `randomDrop`

This filter will randomly drop a message, the drop probability is specified, and is modeled as a uniform distribution between zero and one.

```json
...
   "operation": "random_drop",
	"properties": {
		"name": "prob", 
		"value": 0.5, 
	},
...
```

##### `clone` 

This filter will copy a message and send it to the original destination plus a new one.

```json
...
   "operation": "clone",
	"properties": {
		"name": "add delivery", 
		"value": "endpoint name", 
	},
...
```

##### `firewall`

The firewall filter will eventually be able to execute firewall like rules on messages and perform certain actions on them, that can set flags, or drop or reroute the message. The nature of this is still in development and will be available at a later release.




