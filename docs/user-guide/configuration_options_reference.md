# Configuration Options Reference

```eval_rst
.. toctree::
    :hidden:
    :maxdepth: 2
```

Many of the HELICS entities have significant configuration options. Rather than comprehensively list these options while explaining the features themselves, we've created this section of the User Guide to serve as a reference as to what they are, what they do, and how to use them. This reference is oriented-around the use of JSONs for configuration and is an attempt to be comprehensive in listing and explaining those options. As will be explained below, many of these options are accessible via direct API calls though some of these calls are general in nature (such as [helicsFederateInfoSetIntegrerProperty](https://docs.helics.org/en/v3userguide/c-api-reference/index.html) to set the logging level, among other things). As such

## Configuration methods

Generally, there are three ways in which a co-simulation can be configured and all the various options can be defined:

1. Using direct API calls in the federate source code.
2. Using command-line switches/flags which beginning execution of the federate
3. Using a JSON configuration file (and calling helicsCreateValueFederateFromConfig, helicsCreateMessageFederateFromConfig, or helicsCreateComboFederateFromConfig)

Not all configuration options are available in all three sforms but often they are. For example, it is not possible (nor convenient) to configure a publication for a federate from the command line but it is possible to do so with the JSON config file and with API calls. 

### Choosing configuration method
Which method you use to configure your federate and co-simulation significantly depends on the circumstances of the co-simulation and details of any existing code-base being used. Here is some guidance, though, to help in guiding you're decision in how to do this:

* **If possible, use a JSON configuration file** - Using a JSON configuration file allows creates separation between the code base of the federation and its use in a particular co-simulation. This allows for a modularity between the functionality the federate provides and the particular co-simulation in which it is applied.  For example, a power system federate can easily be reconfigured to work on one model vs another through the use of a JSON configuration file. The particular publications and subscriptions may change but the main functionality of the federate (solving the power flow) does not. 
To use the JSON file for configuration, one of three specific APIs needs to be called: in the file: 
  * `helicsCreateValueFederateFromConfig`
 [C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#ad5dc3f4a7034ae800c67946faf2ce839)
 | [C](https://docs.helics.org/en/v3userguide/c-api-reference/index.html#others)
 | [Python](https://python.helics.org/api/capi-py/#helicsCreateValueFederateFromConfig)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsCreateValueFederateFromConfig-Tuple{String})
  * `helicsCreateMessageFederateFromConfig`
 [C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a8d992360400e996c083c0b3a1d75b8f0)
 | [C](https://docs.helics.org/en/v3userguide/c-api-reference/index.html#others)
 | [Python](https://python.helics.org/api/capi-py/#helicsCreateMessageFederateFromConfig)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsCreateMessageFederateFromConfig-Tuple{String})
  *  `helicsCreateCombinationFederateFromConfig`
 [C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a758381aad8bd5f9d0925a8b13ee04a1d)
 | [C](https://docs.helics.org/en/v3userguide/c-api-reference/index.html#others)
 | [Python](https://python.helics.org/api/capi-py/#helicsCreateCombinationFederateFromConfig)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsCreateCombinationFederateFromConfig-Tuple{String})
* **JSON configuration produces a natural artifact that defines the co-simulation** - Another advantage of the external configuration in the JSON file is that it is a human-readable artifact that can be distributed separately from the source code that provides a lot of information about how the co-simulation was run. In fact, its possible to just look at the configuration files of a federation and do some high-level debugging (checking to see that the subscriptions and publications are aligned, for example).
* **New federates in ill-defined use cases may benefit from API configuration** - The modularity that the JSON config provides may not offer many benefits if the federate is newly integrated into HELICS and/or is part of an evolving analysis. During these times the person(s) doing the integration may just want to make direct API calls instead of having to mess with writing the federate code and a configuration file. There will likely be a point in the future when the software is more codified and switching to a JSON configuration makes more sense. 
* **Command-line configuration (where possible) allows for small, quick changes to the configuration** - Because the command line doesn't provide comprehensive access to the necessary configuration, it will never be a stand-alone configuration option but it does have the advantage of providing quick access right as a user is instantiating the federate. This is an ideal place to make small changes to the configuration (e.g. changing the minimum time step) without having to edit any files. 
* **API configuration is most useful for dynamic configuration** - If there is a need to change the configuration of a given federate dynamically, the API is the only way to do that. Such needs are not common but there are cases where, for example, it may be necessary to define the configuration based on the participants in the federation (_e.g._ publications, subscriptions, timing). It's possible to use [queries](./user_guide/queries.md) to understand the composition and configuration of the federation and then use the APIs to define the configuration of the federate in question. 

### How to Use This Reference
The remainder of this reference lists the configuration options that are supported in the JSON configuration file. Where possible, the corresponding C++ API calls and the links to that documentation will be provided. Generally, the command-line options use the exact same syntax as the JSON configuration options preceded by a `--` and followed by either an `=` or a space and then the parameter value (_i.e._ `--name testname`). In the cases where a single letter switch is available, that will be listed (_i.e._ `-n testname`).

Default values are shown in "[]" following the name(s) of the option.

When an API exists, its name is shown along with links to the specific API documentation for a few (but, sadly, not all) of the supported languages. Many of the options are set with generic functions (_e.g._ `helicsFederateInfoSetFlagOption`) and in those cases the option being set is specified by an enumerated value. In C, these values (_e.g._ `helics_flag_uninterruptible`) are set to integer value (_e.g._ `1`); in this document that integer value follows the enumeration string in brackets. If using the C interface, the integer value must be used. The C++ interface supports the use of the enumerated value directly as do the Python and Julia interfaces with slight syntactical variations (Python: `helics.HELICS_FLAG_INTERRUPTIBLE` and Julia: `HELICS.HELICS_FLAG_INTERRUPTIBLE`).

x

## Sample Configurations
The JSON configuration file below shows all the configuration options in a single file along with their default values and shows what section of the file they should be placed in. Most JSON configuration files will require far fewer options than shown here; items marked with "**" are required. Many items have alternative names that are 

Many of the configuration parameters have alternate names that provide the same functionality. Only one of the names is shown in this configuration file but the alternative names are listed in the reference below. Generally, the supported names are the same string in nocase, camelCase, and snake_case.

An example of one publication, subscription, named input, endpoint, and filter is also shown. The values for each of these options is arbitrary and in the case of filters, many more values are supported and a description of each is provided.

(Note that JSON does not support comments and thus the block below is not valid JSON.)

```json
{
  // General
  **"name": "arbitrary federate name",**
  "core_type": "zmq",
  "core_name": "core name",
  "core_init_string" : "",
  "autobroker": false,
  "connection_required": false,
  "connection_optional": false,
  "strict_input_type_checking": false,
  "terminate_on_error": false,
  "source_only": false,
  "observer": false,
  "only_update_on_change": false,
  "only_transmit_on_change": false,
  
  //Logging
  "logfile": "output.log"
  "log_level": 5,
  "force_logging_flush": false,
  "file_log_level": "",
  "console_log_level": "",
  "dump_log": false,
  
  //Timing
  "ignore_time_mismatch_warnings": false,
  "uninterruptible": false,
  "period": 0,
  "offset": 0,
  "time_delta": 0,
  "minTimeDelta": 0,
  "input_delay": 0,
  "output_delay": 0,
  "real_time": false,
  "rt_tolerance": 0.2,
  "rt_lag": 0.2,
  "rt_lead": 0.2,
  "wait_for_current_time_update": false,
  "restrictive_time_policy": false,
  "slow_responding": false,

  //Iteration
  "rollback": false,
  "max_iterations": 10,
  "forward_compute": false,
  
  
  //Network
  "interfaceNetwork": "local",
  "brokeraddress": "127.0.0.1"
  "reuse_address": false,
  "noack": false,
  "maxsize": 4096,
  "maxcount": 256,
  "networkretries": 5,
  "osport": false,
  "brokerinit": "",
  "server_mode": "",
  "interface": (local IP address),
  "port": 1234,
  "brokerport": 22608,
  "localport": 8080,
  "portstart": 22608,
   
  
  "publications" | "subscriptions" | "inputs": [
    {
      **"key": "publication key",**
      "type": "",
      "unit": "m",
      "global": false,
      "connection_optional": true, 
      "connection_required": false,
      "tolerance": -1,
      "targets": "",
      "buffer_data":  false, indication the publication should buffer data
      "strict_input_type_checking": false,
      "alias": "",
      "ignore_unit_mismatch": false,
      "info": "",
    },
  ],
  "publications" :[
  	{
  		"only_transmit_on_change": false,
  	}
  ]	,
  "subscriptions": [
    {
      "only_update_on_change": false,
    }
  ],
  "inputs": [
    {
      "connections": 1,
      "input_priority_location": 0,
      "clear_priority_list": possible to have this as a config option?
      "single_connection_only": false,
      "multiple_connections_allowed": false
      "multi_input_handling_method": "average",
      "targets": ["pub1", "pub2"]
    }
  ],
  "endpoints": [
    {
      "name": "endpoint name",
      "type": "endpoint type",
      "global": true,
      "destination" | "target" : "default endpoint destination",
      "alias": "",
      "subscriptions": "",
      "filters": "",
      "info": ""
    }
  ],
  "filters": [
    {
      "name": "filter name",
      "source_targets": "endpoint name",
      "destination_targets": "endpoint name",
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
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetCoreName)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetCoreName-Tuple{HELICS.FederateInfo,String})

Every federate must have a unique name across the entire federation; this is functionally the address of the federate and is used to determine where HELICS messages are sent. An error will be generated if the federate name is not unique.

---
	
	
	
### `core_type` | `coretype` | `coreType` | `-t` ["zmq"]
_API:_ `helicsFederateInfoSetCoreTypeFromString`
[C++](https://docs.helics.org/en/latest/doxygen/classhelicscpp_1_1FederateInfo.html#a94654cba67de8d4aaf47cd99bbbd5d60)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetCoreTypeFromString)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetCoreType-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_core_type}})

There are a number of technologies or message buses that can be used to send HELICS messages among federates. Every HELICS enabled simulator has code in it that creates a core which connects to a HELICS broker using one of these messaging technologies. ZeroMQ (zmq) is the default core type and most commonly used but there are also cores that use TCP and UDP networking protocols directly (forgoing ZMQ's guarantee of delivery and reconnection functions), IPC (uses Boost's interprocess communication for fast in-memory message-passing but only works if all federates are running on the same physical computer), and MPI (for use on HPC clusters where MPI is installed). See the [User Guide page on core types](./user-guide/CoreTypes.md) for more details.

---

	

### `core_name` | `corename` | `coreName` []
_API:_ helicsFederateInfoSetCoreName
[C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a1fc4b4563bd06ac54d9569d1df5f8d0c)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetCoreName)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetCoreName-Tuple{HELICS.FederateInfo,String})

Only applicable for `ipc` and `test` core types; otherwise can be left undefined.

---
	
	

### `core_init_string` | `coreinitstring` | `coreInitString` | `-i` []
_API:_ `helicsFederateInfoSetCoreInitString`
[C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a472ea0a8ff1a57d91bfa01b04137e2a8)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetCoreInitString)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetCoreInitString-Tuple{HELICS.FederateInfo,String})

A command-line-like string that specifies options for the core as it connects to the federation. These options are:

- `--broker=` | `broker_address=` | `brokeraddress`: IP address of broker
- `--brokerport=`: Port number on which the broker is communicating
- `--broker_rank=`: For MPI cores only; identifies the MPI rank of the broker
- `--broker_tag=`: For MPI cores only; identifies the MPI tag of the broker
- `--localport=`: Port number to use when communicating with this core
- `--autobroker`: When included the core will automatically generate a broker
- `--key=`: Specifies a key to use when communicating with the broker. Only federates with this key specified will be able to talk to the broker with the same `key` value. This is used to prevent federations running on the same hardware from accidentally interfering with each other.

In addition to these options, all options shown in the `broker_init_string` are also valid.

---
	
	

### `autobroker` [false]
_API:_ (none)

Automatically generate a broker if one cannot be connected to. For federations with only one broker (often the case) and/or with federations containing custom federates that were developed for this particular application, it can be convenient to create the broker in the process of creating a specific federate; this option allows that to take place. The downside to this is it creates a federation with a small amount of mystery as the broker is not clearly shown to be launched as its own federate alongside the other federates and those unfamiliar with the federation composition may have to spend some extra time to understand where the broker is coming from.

---
	
	
	

### `broker_init_string` | `brokerinitstring` | `brokerInitString` [""]
_API:_ `helicsFederateInfoSetBrokerInitString`
[C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a1f145624be99cd3261d4ad1314785e2c)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetBrokerInitString)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetBrokerInitString-Tuple{HELICS.FederateInfo,String})

String used to define the configuration of the broker if one is autogenerated. Such configuration typically includes things like broker IP addresses and port numbers. Again, if this is a co-simulation running on a single computer (and is the only HELICS co-simulation running on said computer) the default option is likely to be sufficient. The following options are available for this string:

- `--federates=` - Number of federates joining the federation.
- `--name=` - Name of the broker; can be used by federates to specify which broker to use
- `--max_iterations=` - Maximum iterations allowed when using the re-iteration API
- `--min_broker_count=` - The minimum number of brokers that the co-simulation must have to begin initialization. (This option is not available for cores)
- `--slow_responding` - Removes the requirement for the broker to respond to pings from other entities in the co-simulation in a timely manner and forces the assumption that this broker is still connected to the federation.
- `--restrictive_time_policy` - Forces the broker to use the most restrictive (conservative) timing policy when granting times to federates. Has the potential to increase co-simulation time as time grants may happen later then they actually need to. 
- `--terminate_on_error` - All errors from any member of the federation will cause the broker to terminate the co-simulation for the entire federation.
- `--force_logging_flush` - Force writing to the log after every message.
- `--log_file=` - Name of file use for logging for this broker.
- `--log_level=` - Specifies the level of logging (both file and console) for this broker. 
- `--file_log_level=` - Specifies the level of logging to file for this broker.
- `--console_log_level=` - Specifies the level of logging to file for this broker.
- `--dumplog` - Captures a record of all logging messages and writes them out to file or console when the broker terminates.
- `--tick=` - Heartbeat period in ms. When brokers fail to respond after 2 ticks secondary actions are taking to confirm the broker is still connected to the federation. Times can also be entered as strings such as "15s" or "75ms".
- `--network_timeout=` - Time to establish a socket connection in ms. Times can also be entered as strings such as "15s" or "75ms".
- `--error_timeout=` - Time in ms to wait after an error state is reached before terminating. Times can also be entered as strings such as "15s" or "75ms".

---
	



### `terminate_on_error` | `terminateonerror` | `terminateOnError` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_terminate_on_error` [72]

If the `terminate_on_error` flag is set then a federate encountering an internal error will trigger a global error and cause the entire federation to terminate. Errors of this nature are typically the result of configuration errors, such as having a required publication that is not used or incompatible units or types on publications and subscriptions.

---
	
	

### `source_only` | `sourceonly` | `sourceOnly` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_source_only` [4]

Used to indicate to the federation that this federate is only producing data and has no inputs/subscriptions. Specifying this when appropriate allows HELICS to more efficiently grant times to the federation.

---
	
	
	

### `observer` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_observer` [0]

Used to indicate to the federation that this federate produces no data and only has inputs/subscriptions. Specifying this when appropriate allows HELICS to more efficiently grant times to the federation.



## Logging Options

### `log_file` | `logfile` | `logFile` []
_API:_ `helicsFederateSetLogFile`
[C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#abde89169985b8a18c2d1b8fa803e5169)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federate)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateSetLogFile)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsBrokerSetLogFile-Tuple{HELICS.Broker,String})

Specifies the name of the log file where logging messages will be written.

---
	
	
	

### `log_level` | `loglevel` | `logLevel` [0]
_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#ad6a898deb8df83ee31d62eccbb202aef)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Int64})

_Property's enumerated name:_ `helics_property_int_log_level` [271]

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

---
	



### `file_log_level` | `fileloglevel` | `fileLogLevel` [null]
_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#ad6a898deb8df83ee31d62eccbb202aef)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Int64})

_Property's enumerated name:_ `helics_property_int_file_log_level` [272]

_Valid values:_ Same as in `loglevel`

Allows a distinct log level for the written log file to be specified. By default the logging level to file and console are identical and will only differ if `file_log_level` or `console_log_level` are defined.

---
	
	
	

### `console_log_level` | `consoleloglevel` | `consoleLogLevel` [null]
_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#ad6a898deb8df83ee31d62eccbb202aef)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Int64})

_Property's enumerated name:_ `helics_property_int_console_log_level` [274]

_Valid values:_ Same as in `loglevel`

Allows a distinct log level for the written log file to be specified. By default the logging level to file and console are identical and will only differ if `file_log_level` or `console_log_level` are defined.

---
	


### `force_logging_flush` | `forceloggingflush` | `forceLoggingFlush` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_force_logging_flush` [88]

Setting this option forces HELICS logging messages to be flushed to file after each one is written. This prevents the buffered IO most OSs implement to be bypassed such that all messages appear in the log file immediately after being written at the cost of slower simulation times due to more time spent writing to file.

---
	
	
	

### `dump_log` | `dumplog` | `dumpLog` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_dumplog` [89]

When set, a record of all messages is captured and written out to the log file at the conclusion of the co-simulation.






## Timing Options

### `ignore_time_mismatch` | `ignoretimemismatch` | `ignoreTimeMismatch` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_ignore_time_mismatch_warnings` [67]

If certain timing options (_i.e._ `period`, or `minTimeDelta`) are used it is possible for the time granted a federate to be greater than the requested time. This situation would normally generate a warning message, but if this flag is set those warnings are silenced. 

---
	


### `uninterruptible` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_uninterruptible` [1]

Normally, a federate will be granted a time earlier than it requested when it receives a message from another federate; the presence of any message implies there could be an action the federate needs to take and may generate new messages of its own. There are times, though, when it is important that the federate only be granted a time (and begin simulating/executing again) that it has previously requested. For example, there could be some controller that should only operate at fixed intervals even if new data arrives earlier. In these cases, setting the `uninterruptible` flag will prevent premature time grants.

---
	


### `period` [0]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Union{Float64,%20Int64}})

_Property's enumerated name:_ `helics_property_time_period` [140]

Many time-based simulators have a minimum time-resolution or a user-configurable step size. The `period` parameter can be used to effectively synchronize the times that are granted with the defined simulation period. The default units for `period` are in seconds but the string for this parameter can include its own units (e.g. "2 ms" or "1 hour"). Setting `period` will force all time grants to occur at times of `n*period` even if subscriptions are updated, messages arrive, or the federate requests a time between periods. This value effectively makes the federates `uninterruptible` during the times between periods. Relatedly...

---
	
	

### `offset` [0]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Union{Float64,%20Int64}})

_Property's enumerated name:_ `helics_property_time_offset` [141]

There may be cases where it is preferable to have a simulator receive time grants that are offset slightly in time to one or more other federates. Defining an `offset` value allows this to take place; units are handled the same as in `period`. Setting both `period` and `offset`, will result in the all times granted to the federate in question being constrained to `n*period + offset`.

---
	


### `time_delta` | `timeDelta` | `timedelta` [1ns]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Union{Float64,%20Int64}})

_Property's enumerated name:_ `helics_property_time_delta` [137]

timeDelta has some similarities to `period`; where `period` constrained the granted time to regular intervals, `timeDelta` constrains the grant time to a minimum amount from the last granted time. Units are handled the same as in `period`.

---
	



### `input_delay` | `inputdelay` | `inputDelay` [0]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Union{Float64,%20Int64}})

_Property's enumerated name:_ `helics_property_time_input_delay` [148]

`inputDelay` specifies a delay in simulated time between when a signal arrives at a federate and when that federate is notified that a new value is available. `outputDelay` is similar but applies to signals being sent by a federate. Note that this applies to both value signals and message signals.

---
	

### `output_delay` | `outputdelay` | `outputDelay` [0]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Union{Float64,%20Int64}})

_Property's enumerated name:_ `helics_property_time_output_delay` [150]

`outputDelay` is similar to `input_delay` but applies to signals being sent by a federate. Note that this applies to both value signals and message signals.

---
	
	


### `real_time` | `realtime` | `realTime` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_realtime` [16]

If set to true the federate uses `rt_lag` and `rt_lead` to match the time grants of a federate to the computer wall clock.
If the federate is running faster than real time this will insert additional delays. If the federate is running slower than real time this will cause a force grant, which can lead to non-deterministic behavior. `rt_lag` can be set to maxVal to disable force grant

---



	
### `rt_lag`| `rtlag` | `rtLag` [0.2] and `rt_lead` | `rtlead` | `rtLead` [0.2]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Union{Float64,%20Int64}})

_Property's enumerated name:_ `helics_property_time_rt_lag` [143] and `helics_property_time_rt_lead` [144]

Defines "real-time" for HELICS by setting tolerances for HELICS to use when running a real-time co-simulation. HELICS is forced to keep simulated time within this window of wall-clock time. Most general purpose OSes do not provide guarantees of execution timing and thus very small values of `rt_lag` and `rt_lead` (less than 0.005) are not likely to be achievable.

---
	


### `rt_tolerance` | `rttolerance` | `rtTolerance` [0.2]
_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#aef32f6cb11188baf60cc8826914a4b6f)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Union{Float64,%20Int64}})

_Property's enumerated name:_ `helics_property_time_rt_tolerance` [145]

Implements the same functionality of `rt_lag` and `rt_lead` but does so by using a single value to set symmetrical lead and lag constraints. 

---

	


### `wait_for_current_time_update` |`waitforcurrenttimeupdate` | `waitForCurrentTimeUpdate` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_wait_for_current_time_update` [10]

If set to true, a federate will not be granted the requested time until all other federates have completed at least 1 iteration of the current time or have moved past it. If it is known that 1 federate depends on others in a non-cyclic fashion, this can be used to optimize the order of execution without iterating.

---
	
	

### `restrictive_time_policy` | `restrictivetimepolicy` | `restrictiveTimePolicy` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_restrictive_time_policy` [11]

If set, a federate will not be granted the requested time until all other federates have completed at least 1 iteration of the current time or have moved past it. If it is known that 1 federate depends on others in a non-cyclic fashion, this can be used to optimize the order of execution without iterating.

Using the option `restrictive-time-policy` forces HELICS to use a fully conservative mode in granting time. This can be useful in situations beyond the current reach of the distributed time algorithms. It is generally used in cases where it is known that some federate is executing and will trigger someone else, but most federates won't know who that might be. This prevents extra messages from being sent and a potential for time skips. It is not needed if some federates are periodic and execute every time step. The flag can be used for federates, brokers, and cores to force very conservative timing with the potential loss of performance as well.

Only applicable to Named Input interfaces ([see section on value federate interface types](./user-guide/value_federates.md)), if enabled this flag checks that data type of the incoming signals match that specified for the input. 

---
	


### `slow_responding` | `slowresponsing` | `slowResponding` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_slow_responding` [29]

If specified on a federate, setting this flag indicates the federate may be slow in responding, and to not forcibly eject the federate from the federation for the slow response. This is an uncommon scenario.

If applied to a core or broker (`--slow_responding` in the `core_init_string` or `broker_init_string`, respectively), it is indicative that the broker doesn't respond to internal pings quickly and should not be disconnected from the federation for the slow response.












## Iteration

### `forward_compute` | `forwardcompute` | `forwardCompute` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_forward_compute [14]

Indicates to the broker and the rest of the federation that this federate computes ahead of its granted time and can/does roll back when necessary. Federates able to do this (and who set this flag) allow more efficient time grants to the federation as a whole.

---
	


### `rollback` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_rollback` [12]

Indicates to the broker and the rest of the federation that this federate can/does roll back when necessary. Federates able to do this (and who set this flag) allow more efficient time grants to the federation as a whole.

---


	

### `max_iterations` | `maxiterations` | `maxIteration` [50]
_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#ad6a898deb8df83ee31d62eccbb202aef)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_properties},Int64})

_Property's enumerated name:_ `helics_property_int_max_iterations` [259]

For federates engaged in iteration (recomputing values based on updated inputs at a single simulation timestep) there may be a need to enforce a maximum number of iterations. This option allows that value to be set. When any federate reaches this number of iterations, HELICS will evaluate the federation as a whole and grant the next smallest time supported by the iterating federates. This time will only be granted to the federates that would be able to execute at this time.











## General and Per Subscription, Input, or Publication
These options can be set globally for all subscriptions, inputs and publications for a given federate. Even after setting them globally, they can be included in the configuration for an individual subscription, input, or publication, over-riding the global setting.


### `only_update_on_change` | `onlyupdateonchange` | `onlyUpdateOnChange` [false] and `only_transmit_on_change` | `onlytransmitonchange` | `onlyTransmitOnChange` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_flag_only_update_on_change` [454] and `helics_flag_only_transmit_on_change` [452]

Setting these flags prevents new value signals with the same value from being received by the federate or sent by the federate. Setting these flags will reduce the amount of traffic on the HELICS bus and can provide performance improvements in co-simulations with large numbers of messages.

---
	


### `tolerance`
_API:_ `helicsPublicationSetMinimumChange` and `helicsInputSetMinimumChange`
[C++ input](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Input.html#a55056ac9dd2895270f575827dd9951c7) and [C++ publication](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Publication.html#ab66f5680bb4a5e062314f6f8e5dea846)
 | [C input](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo) and [C publication](https://docs.helics.org/en/latest/c-api-reference/index.html#publication)
 | [Python input](https://python.helics.org/api/capi-py/#helicsInputSetMinimumChange) and [Python publication](https://python.helics.org/api/capi-py/#helicsPublicationSetMinimumChange)
 | [Julia input](https://julia.helics.org/latest/api/#HELICS.helicsInputSetMinimumChange-Tuple{HELICS.Subscription,Float64}) and [Julia publication](https://julia.helics.org/latest/api/#HELICS.helicsPublicationSetMinimumChange-Tuple{HELICS.Publication,Float64})

This option allows the specific numerical definition of "change" when using the `only_update_on_change` and `only_transmit_on_change` options.

---
	
	
	

### `connection_required` | `connectionrequired` | `connectionRequired` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_handle_option_connection_required` [397]


When a federate is initialized, one of its tasks is to make sure the recipients of directed signals exist. If, after the federation is initialized, the recipient can't be found, then by default a warning is generated and written to the log file. If the `connections_required` flag is set, this warning becomes a fatal error that stops the co-simulation. 

- `publications` - At least one federate must subscribe to the publications.
- `subscriptions` - The message being subscribed to must be provided by some other publisher in the federation.

---

	

### `connection_optional` | `connectionoptional` | `connectionOptional` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_handle_option_connection_optional` [402]

When an interface requests a target it tries to find a match in the federation. If it cannot find a match at the time the federation is initialized, then the default is to generate a warning. This will not halt the federation but will display a log message. If the `connections_optional` flag is set on a federate all subsequent `addTarget` calls on any interface will not generate any message if the target is not available.













## Subscription, Input, and/or Publication Options
These options are valid for subscriptions, inputs, and/or publications (generically called "handles"). The APIs for dealing with registering these handles combine multiple options in the JSON config file and have varying levels of specificity (defining the date type for the handle or defining the handle as global). Rather than listing all APIs for the following options, the main APIs will be listed here and those using them can consult the API references to see which specific APIs are most applicable.

`helicsFederateRegisterPublication`
[C++](https://docs.helics.org/en/latest/doxygen/ValueFederate_8h.html#ac00c4e17aeb9e20fdf0f42fb8bc63d29)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federate)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterPublication)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterPublication)

`helicsFederateRegisterSubscription`
[C++](https://docs.helics.org/en/latest/doxygen/ValueFederate_8h.html#ab30020ca06ad37548691f313df42e15f)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federate)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterSubscription)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterSubscription)

`helicsFederateRegisterInput`
[C++](https://docs.helics.org/en/latest/doxygen/ValueFederate_8h.html#aa41b313f5e527055444bf915c8da3258)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federate)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterInput)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterInput)



### `key` (required)

- `publications` - The string in this field is the unique identifier (at the federate level) for the value that will be published to the federation. If `global` is set (see below) it must be unique to the entire federation.
- `subscriptions` - This string identifies the federation-unique value that this federate wishes to receive. Unless `global` has been set to `true` in the publishings JSON configuration file, the name of the value is formatted as `<federate name>/<publication key>`. Both of these strings can be found in the publishing federate's JSON configuration file as the `name` and `key` strings, respectively. If `global` is `true` the string is just the `key` value.
- `input` - The string in this field is the unique identifier (at the federate level) that defines the input to receive value signals. 

---
	
	

### `type` [null]

HELICS supports data types and data type conversion ([as best it can](https://www.youtube.com/watch?v=mZOAn-3aATY)). 

---
	



### `unit` [null]

HELICS is able to do some levels of unit conversion, currently only on double type publications but more may be added in the future. The units can be any sort of unit string, a wide assortment is supported and can be compound units such as m/s^2 and the conversion will convert as long as things are convertible. The unit match is also checked for other types and an error if mismatching units are detected. A warning is also generated if the units are not understood and not matching. The unit checking and conversion is only active if both the publication and subscription specify units.

---
	


### `global` [false]

(publications only) `global` is used to indicate that the value in `key` will be used as a global name when other federates are subscribing to the message. This requires that the user ensure that the name is used only once across all federates. Setting `global` to `true` is handy for federations with a small number of federates and a small number of message exchanges as it allows the `key` string to be short and simple. For larger federations, it is likely to be easier to set the flag to `false` and accept the extra naming.

---
	


### `buffer_data` | `bufferdata` | `bufferData` [false]
_API:_ `helicsInputSetOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](https://docs.helics.org/en/latest/c-api-reference/index.html#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Subscription,Union{Int64,%20HELICS.Lib.helics_handle_options},Bool})

_Property's enumerated name:_ `helics_handle_option_buffer_data` [411]

(only valid for inputs and subscriptions) Setting this flag will buffer the last value sent during the initialization phase of HELICS co-simulations. When the execution phase begins, that value will be resent to the receiving handle.

---
	


### `alias` [null]
_API:_
xxxxxxx - Is this a user-facing option? The API is a part of the application API but doesn't start "helics..." making me wonder.

---
	



### `ignore_units_mismatch | ignoreunitmismatch | ignoreUnitMismatch` [null]
_API:_ 
Under normal operation, handles that are connected (value signals flowing between them) are required to have units that either match or can be directly converted between. If mismatching units are connected, an error is thrown; when this flag is set that error is suppressed.

---
	


### `info` [""]
_API:_ `helicsInputSetInfo` 
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga8117e8d7c987b3fb27e065b6693116e6)
| [C](https://docs.helics.org/en/latest/c-api-reference/index.html#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetInfo)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetInfo-Tuple{HELICS.Subscription,String})
The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example.

---
	

### `strict_input_type_checking` | `strictinputtypechecking` | `strictInputTypeChecking` [false]
_API:_ `helicsFederateInfoSetFlagOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federateinfo)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `helics_handle_option_strict_type_checking` [414]

When an interface requests a target it tries to find a match in the federation. If it cannot find a match at the time the federation is initialized, then the default is to generate a warning. This will not halt the federation but will display a log message. If the `connections_optional` flag is set on a federate all subsequent `addTarget` calls on any interface will not generate any message if the target is not available.x







## Input-only Options
Inputs can receive values from multiple sending handles and the means by which those multiple data points for a single handle are managed can be specified with several options. See the [User Guide entry](./user-guide/multiSourceInputs.md) for further details.


### `connections` []
_API:_ `helicsInputSetOption` 
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](https://docs.helics.org/en/latest/c-api-reference/index.html#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Subscription,Union{Int64,%20HELICS.Lib.helics_handle_options},Bool})

_Property's enumerated name:_ `helics_handle_option_connections ` [522]

Allows an integer number of connections to be considered value for this input handle. Similar to `multiple_connections_allowed` but allows the number of sending handles to be defined to a specific number.

---
	
	

### `input_priority_location` | `inputprioritylocation` | `inputPriorityLocation` []
_API:_ `helicsInputSetOption` 
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga5cb76ace18e1fa7cedd1d5ed8a56f3d1)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#input)
 | [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Subscription,Union{Int64,%20HELICS.Lib.helics_handle_options},Bool})
 
 _Property's enumerated name:_ `helics_handle_option_input_priority_location` [510] 
 
When receiving values from multiple sending handles, when the values are received they or organized as a vector. This option is used to define which value in that vector has priority. The API can be called multiple times to set successive priorities. 


---
	
	

### `clear_priority_list` | `clearprioritylist` | `clearPriorityList` [false]
_API:_ `helicsInputSetOption` 
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga5cb76ace18e1fa7cedd1d5ed8a56f3d1)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#input)
 | [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Subscription,Union{Int64,%20HELICS.Lib.helics_handle_options},Bool})
 
 _Property's enumerated name:_ `helics_handle_option_clear_priority_list` [512] 

When receiving values from multiple sending handles, when the values are received they or organized as a vector. This option is used to clear that priority list and redefine which values have priority.

---
	


### `single_connection_only` | `singleconnectiononly` |`singleConnectionOnly` [false]
_API:_ `helicsInputSetOption` 
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga5cb76ace18e1fa7cedd1d5ed8a56f3d1)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#input)
 | [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Subscription,Union{Int64,%20HELICS.Lib.helics_handle_options},Bool})
 
 _Property's enumerated name:_ `helics_handle_option_single_connection_only ` [407] 
 When set, this forces the input handle to have only one sending handle it will receive from. Setting this flag serves as a sort of double-check to ensure that only one other handle is sending to this input handle and that the federation has been configured as expected.
 
 ---
	


### `multiple_connections_allowed` | `multipleconnectionsallowed` | `multipleConnectionsAllowed` [true]
_API:_ `helicsInputSetOption` 
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga5cb76ace18e1fa7cedd1d5ed8a56f3d1)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#input)
 | [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Subscription,Union{Int64,%20HELICS.Lib.helics_handle_options},Bool})

_Property's enumerated name:_ `helics_handle_option_multiple_connections_allowed` [409] 
 When set, this flag allows the input handle to receive valuesfrom multiple other handles. 
 
 ---
	
	


### `multi_input_handling_method` | `multiinputhandlingmethod` | `multiInputHandlingMethod` [`none`]
_API:_ `helicsInputSetOption` 
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga5cb76ace18e1fa7cedd1d5ed8a56f3d1)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#input)
 | [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Subscription,Union{Int64,%20HELICS.Lib.helics_handle_options},Bool})

_Property's enumerated name:_ `helics_handle_option_multi_input_handling_method` [507] 
_Property values:_
* `none` | `no_op`
* `or`
* `sum`
* `max`
* `min`
* `average`
* `mean`
* `vectorize`
* `diff`

Given that an input can have multiple data sources, a method of reducing those multiple values into one needs to be defined. HELICS supports a number of mathematical operation to perform this reduction.

---




## Endpoint Options
As in the value handles, the registration of endpoints is done through a single API that incorporates multiple options. And as in the value handles, there is a `global` API option to allow the name of the endpoint to be considered global to the federation. 

_API:_ `helicsFederateRegisterEndpoint` 
[C++](https://docs.helics.org/en/latest/doxygen/MessageFederate_8h.html#a2eb3f4680791a817b7654f0b6ca97d4d)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federate)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterEndpoint)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterEndpoint-Tuple{HELICS.Federate,String,String})



### `name` (required)
The name of the endpoint as it will be known to the rest of the federation.

---
	
	

### `type` []
_API:_ (none)

xxxxxxx

---
	
	

### `destination` | `target` []
_API:_ `helicsEndpointSetDefaultDestination` 
[C++](https://docs.helics.org/en/latest/doxygen/MessageFederate_8h.html#a4d32466958d0b47ded8825380275d787)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#endpoint)
 | [Python](https://python.helics.org/api/capi-py/#helicsEndpointSetDefaultDestination)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsEndpointSetDefaultDestination-Tuple{HELICS.Endpoint,String})

Defines the default destination for a message sent from this endpoint.

---

	

### `alias` []
_API:_ (none)

Creates a local alias for a handle that may have a long name.

---


	

### `subscriptions` []
_API:_ `helicsEndpointSubscribe` 
[C++](https://docs.helics.org/en/latest/doxygen/MessageFederate_8h.html#ae7ab88b9e49dc6c3ef5f2042b1890a45)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#endpoint)
 | [Python](https://python.helics.org/api/capi-py/#helicsEndpointSubscribe)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsEndpointSubscribe-Tuple{HELICS.Endpoint,String})

---
	
	

### `filters` [null]
See section on Filter Options.


---



### `info` [""]
_API:_ `helicsEndpointSetInfo` 
[C++](https://docs.helics.org/en/latest/doxygen/MessageFederate_8h.html#acb58eb5e9fca5c05451592d4a76de524)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#endpoint)
 | [Python](https://python.helics.org/api/capi-py/#helicsEndpointSetInfo)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsEndpointSetInfo-Tuple{HELICS.Endpoint,String})
The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example.



## Filter Options
Filters are registered with the core or through the application API.
There are also Filter object that hide some of the API calls in a slightly nicer interface. Generally a filter will define a target endpoint as either a source filter or destination filter. Source filters can be chained, as in there can be more than one of them. At present there can only be a single non-cloning destination filter attached to an endpoint.

Non-cloning filters can modify the message in some ways, cloning filters just copy the message and may send it to multiple destinations.

On creation, filters have a target endpoint and an optional name.
Custom filters may have input and output types associated with them.
This is used for chaining and automatic ordering of filters.
Filters do not have to be defined on the same core as the endpoint, and in fact can be anywhere in the federation, any messages will be automatically routed appropriately.

_API:_ `helicsFederateRegisterFilter` 
([C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#ae51e3c5dc5a974b3f1ec4c37e4901580)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federate)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterFilter)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterFilter-Tuple{HELICS.Federate,Union{Int64,%20HELICS.Lib.helics_filter_type},String}))
 to create/register the filter and then `helicsFilterAddSourceTarget`
 ([C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#ae3c07304f81645db5f92fd4a3b9e53ce)
 | [C](https://docs.helics.org/en/v3userguide/c-api-reference/index.html#filter)
 | [Python](https://python.helics.org/api/capi-py/#helicsFilterAddSourceTarget)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFilterAddSourceTarget-Tuple{HELICS.Filter,String}))
 or `helicsFilterAddDestinationTarget` 
  ([C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#a664e4b45227a9e1070951376836eb8d6)
 | [C](https://docs.helics.org/en/v3userguide/c-api-reference/index.html#filter)
 | [Python](https://python.helics.org/api/capi-py/#helicsFilterAddDestinationTarget)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFilterAddDestinationTarget-Tuple{HELICS.Filter,String}))
 to associate it with a specific endpoint



### `name` []
_API:_ (none)

Name of the filter; must be unique to a federate. 

---

	

### `source_targets`, `sourcetargets`, `sourceTargets` []
_API:_ `` 
[C++]()
 | [C]()
 | [Python](https://python.helics.org/api/capi-py/#helicsFilterAddSourceTarget)
 | [Julia]()

Acts on previously registered filter and associated with a specific endpoint of the federate.

---
	


### `destination_targets`, `destinationtargets`, `destinationtargets` []
_API:_ `` 
[C++]()
 | [C]()
 | [Python](https://python.helics.org/api/capi-py/#helicsFilterAddDestinationTarget)
 | [Julia]()

Acts on previously registered filter and associated with a specific endpoint of the federate.

---
	


### `info` [""]
_API:_ `helicsFilterSetInfo` 
[C++](https://docs.helics.org/en/latest/doxygen/group__Clone.html#gaf8d846ddb206fdbc7c22769f4a2b2f9b)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#filter)
 | [Python](https://python.helics.org/api/capi-py/#helicsFilterSetInfo)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFilterSetInfo-Tuple{HELICS.Filter,String}})
The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example.

---


	
### `operation` []
_API:_ `helicsFederateRegisterFilter` 
[C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#ae51e3c5dc5a974b3f1ec4c37e4901580)
 | [C](https://docs.helics.org/en/latest/c-api-reference/index.html#federate)
 | [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterFilter)
 | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterFilter-Tuple{HELICS.Federate,Union{Int64,%20HELICS.Lib.helics_filter_type},String})

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





## Network

For most HELICS users, most of the time, the following network options are not needed. They are most likely to be needed when working in complex networking environments, particularly when running co-simulations across multiple sites with differing network configurations. Many of these options require non-trivial knowlege of network operations and rather and it is assumed that those that needs these options will understand what they do, even with the minimal descriptions given.


### interface network
_API:_ 
See multiple options for --local, --ipv4, --ipv6, --all

---


### `reuse_address` | `reuseaddress` | `reuseAddress` [false]
_API:_ (none)

Allows the server to reuse a bound address, mostly useful for tcp cores.

---
	
	

### `noack_connect` | `noackconnect` | `noackConnect` [false]

Specify that a connection_ack message is not required to be connected with a broker.

---
	


### `max_size` | `maxsize` | `maxSize`[4096]
_API:_ (none)

Message buffer size. Can be increased for large messages to limit the number of retries by the underlying networking protocols. 

---
	


### `max_count` | `maxcount` | `maxCount` [256]
_API:_ (none)

Maximum number of messages in queue. Can be increased for large volumes of messages to limit the number of retries by the underlying networking protocols. 

---

	

### `network_retries` | `networkretries` | `networkRetries` [5]
_API:_ (none)
Maximum number of network retry attempts. 

---

	

### `use_os_port` | `useosport` | `useOsPort` [false]
_API:_ (none)
Setting this flag specifies that the OS should set the port for the HELICS message bus. HELICS will ask the operating system which port to use and will use the indicated port.

---
	
	

### `client` or `server` [null]
_API:_ (none)
specify that the network connection should be a server or client. By default neither option is enabled. 

---
	
	
	
### `local_interface` | `localinterface` | `localInterface` [local address]
_API:_ (none)
the local interface to use for the receive ports. 

---
	
	
	

### `port` | `-p` []
_API:_ (none)
Port number to use. 

---
	
	

### `broker_port` | `brokerport` | `brokerPort` []
_API:_ (none)

The port to use to connect to the broker.

---



### `broker_name` | `brokername` | `brokerName` []
_API:_ (none)

---




### `local_port` | `localport` | `localPort` []
_API:_ (none)
port number for the local receive port.

---

	

### `port_start` | `portstart` | `portStart` []
_API:_ (none)
starting port for automatic port definitions.







