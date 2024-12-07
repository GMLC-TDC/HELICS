# Configuration Options Reference

```{eval-rst}
.. toctree::
    :hidden:
    :maxdepth: 2
```

Many of the HELICS entities have significant configuration options. Rather than comprehensively list these options while explaining the features themselves, we've created this section of the User Guide to serve as a reference as to what they are, what they do, and how to use them. This reference is oriented-around the use of JSON files for configuration and is an attempt to be comprehensive in listing and explaining those options. As will be explained below, many of these options are accessible via direct API calls though some of these calls are general in nature (such as [helicsFederateInfoSetIntegrerProperty](api-reference/C_API.md#federateinfo) to set the logging level, among other things).

## Configuration methods

Generally, there are three ways in which a co-simulation can be configured and all the various options can be defined:

1. Using direct API calls in the federate source code.
2. Using command-line switches/flags while beginning execution of the federate
3. Using a JSON configuration file (and calling helicsCreateValueFederateFromConfig, helicsCreateMessageFederateFromConfig, or helicsCreateComboFederateFromConfig)

Not all configuration options are available in all three forms but often they are. For example, it is not possible (nor convenient) to configure a publication for a federate from the command line but it is possible to do so with the JSON config file and with API calls.

### Choosing configuration method

Which method you use to configure your federate and co-simulation significantly depends on the circumstances of the co-simulation and details of any existing code-base being used. Here is some guidance, though, to help in guiding you're decision in how to do this:

- **If possible, use a JSON configuration file** - Using a JSON configuration file creates separation between the code base of the federation and its use in a particular co-simulation. This allows for a modularity between the functionality the federate provides and the particular co-simulation in which it is applied. For example, a power system federate can easily be reconfigured to work on one model vs another through the use of a JSON configuration file. The particular publications and subscriptions may change but the main functionality of the federate (solving the power flow) does not.
  To use the JSON file for configuration, one of three specific APIs needs to be called: in the file:
  - `helicsCreateValueFederateFromConfig`
    [C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#ad5dc3f4a7034ae800c67946faf2ce839)
    | [C](api-reference/C_API.md#creation)
    | [Python](https://python.helics.org/api/capi-py/#helicsCreateValueFederateFromConfig)
    | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsCreateValueFederateFromConfig-Tuple{String})
  - `helicsCreateMessageFederateFromConfig`
    [C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a8d992360400e996c083c0b3a1d75b8f0)
    | [C](api-reference/C_API.md#creation)
    | [Python](https://python.helics.org/api/capi-py/#helicsCreateMessageFederateFromConfig)
    | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsCreateMessageFederateFromConfig-Tuple{String})
  - `helicsCreateCombinationFederateFromConfig`
    [C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a758381aad8bd5f9d0925a8b13ee04a1d)
    | [C](api-reference/C_API.md#creation)
    | [Python](https://python.helics.org/api/capi-py/#helicsCreateCombinationFederateFromConfig)
    | [Julia](https://julia.helics.org/latest/api/#HELICS.helicsCreateCombinationFederateFromConfig-Tuple{String})
- **JSON configuration produces a natural artifact that defines the co-simulation** - Another advantage of the external configuration in the JSON file is that it is a human-readable artifact that can be distributed separately from the source code that provides a lot of information about how the co-simulation was run. In fact, its possible to just look at the configuration files of a federation and do some high-level debugging (checking to see that the subscriptions and publications are aligned, for example).
- **New federates in ill-defined use cases may benefit from API configuration** - The modularity that the JSON config provides may not offer many benefits if the federate is newly integrated into HELICS and/or is part of an evolving analysis. During these times the person(s) doing the integration may just want to make direct API calls instead of having to mess with writing the federate code and a configuration file. There will likely be a point in the future when the software is more codified and switching to a JSON configuration makes more sense.
- **Command-line configuration (where possible) allows for small, quick changes to the configuration** - Because the command line doesn't provide comprehensive access to the necessary configuration, it will never be a stand-alone configuration option but it does have the advantage of providing quick access right as a user is instantiating the federate. This is an ideal place to make small changes to the configuration (e.g. changing the minimum time step) without having to edit any files.
- **API configuration is most useful for dynamic configuration** - If there is a need to change the configuration of a given federate dynamically, the API is the only way to do that. Such needs are not common but there are cases where, for example, it may be necessary to define the configuration based on the participants in the federation (_e.g._ publications, subscriptions, timing). It's possible to use [queries](../user-guide/advanced_topics/queries.md) to understand the composition and configuration of the federation and then use the APIs to define the configuration of the federate in question.

### How to Use This Reference

The remainder of this reference lists the configuration options that are supported in the JSON configuration file. Where possible, the corresponding C++ API calls and the links to that documentation will be provided. Generally, the command-line options use the exact same syntax as the JSON configuration options preceded by a `--` and followed by either an `=` or a space and then the parameter value (_i.e._ `--name testname`). In the cases where a single letter switch is available, that will be listed (_i.e._ `-n testname`).

Default values are shown in "[]" following the name(s) of the option.

When an API exists, its name is shown along with links to the specific API documentation for a few (but, sadly, not all) of the supported languages. Many of the options are set with generic functions (_e.g._ `helicsFederateInfoSetFlagOption`) and in those cases the option being set is specified by an enumerated value. In C, these values (_e.g._ `helics_flag_uninterruptible`) are set to integer value (_e.g._ `1`); in this document that integer value follows the enumeration string in brackets. If using the C interface, the integer value must be used. The C++ interface supports the use of the enumerated value directly as do the Python and Julia interfaces with slight syntactical variations (Python: `helics.HELICS_FLAG_INTERRUPTIBLE` and Julia: `HELICS.HELICS_FLAG_INTERRUPTIBLE`).

## Sample Configurations

The JSON configuration file below shows all the configuration options in a single file along with their default values and shows what section of the file they should be placed in. Most JSON configuration files will require far fewer options than shown here; items marked with "\*\*" are required.

Many of the configuration parameters have alternate names that provide the same functionality. Only one of the names is shown in this configuration file but the alternative names are listed in the reference below. Generally, the supported names are the same string in nocase, camelCase, and snake_case.

An example of one publication, subscription, named input, endpoint, and filter is also shown. The values for each of these options is arbitrary and in the case of filters, many more values are supported and a description of each is provided.

(Note that the JSON standard does not support comments and thus the block below is not valid JSON. The JSON parser HELICS uses does support comments)

```text
{
  // General
  **"name": "arbitrary federate name",**
  "core_type": "zmq",
  "core_name": "core name",
  "core_init_string" : "",
  "autobroker": false,
  "connection_required": false,
  "connection_optional": true,
  "strict_input_type_checking": false,
  "terminate_on_error": false,
  "source_only": false,
  "observer": false,
  "dynamic":false,
  "only_update_on_change": false,
  "only_transmit_on_change": false,
  "broker_key": "",

  //Logging
  "logfile": "output.log"
  "log_level": "warning",
  "force_logging_flush": false,
  "file_log_level": "",
  "console_log_level": "",
  "dump_log": false,
  "logbuffer": 10,

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
  "grant_timeout": 0,
  "max_cosim_duration":0,
  "wait_for_current_time_update": false,
  "restrictive_time_policy": false,
  "slow_responding": false,

  //Iteration
  "rollback": false,
  "max_iterations": 10,
  "forward_compute": false,

  // other
  "indexgroup":5,

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
  "encrypted": false,
  "encryption_config":"encryption_config.json",


  "publications" | "subscriptions" | "inputs": [
    {
      **"key": "publication key",**
      "type": "",
      "unit": "m",
      "global": false,
      "connection_optional": true,
      "connection_required": false,
      "tolerance": -1,
      // for targets can be singular or plural, if an array must use plural form
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
  ]    ,
  "subscriptions": [
    {
      "only_update_on_change": false,
      "default": value,
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
      // for targets can be singular or plural, if an array must use plural form
      "targets": ["pub1", "pub2"]
      "default": 5.5,
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
  "translators": [
    {
      "name": "translator name",
      // can use singular form if only a single target
      "source_target": "publication name",
      "destination_targets": "endpoint name",
      "info": "",
      "type": "JSON",
    }
  ]
}
```

## General Federate Options

There are a number of flags which control how a federate acts with respect to timing and its signal interfaces.

### `name` | `-n` (required)

_API:_ `helicsFederateInfoSetCoreName`
[C++](https://docs.helics.org/en/latest/doxygen/helics_8h.html#a1fc4b4563bd06ac54d9569d1df5f8d0c)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetCoreName)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetCoreName-Tuple{HELICS.FederateInfo,%20String})

Every federate must have a unique name across the entire federation; this is functionally the address of the federate and is used to determine where HELICS messages are sent. An error will be generated if the federate name is not unique.

---

### `core_type` | `-t` ["zmq"]

_Alternative names:_ `coretype` | `coreType`

_API:_ `helicsFederateInfoSetCoreTypeFromString`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1BasicFedInfo.html#a1597a91e6783094a6cf29bd4d0493785)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetCoreTypeFromString)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetCoreType-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsCoreTypes}})

There are a number of technologies or message buses that can be used to send HELICS messages among federates. Every HELICS enabled simulator has code in it that creates a core which connects to a HELICS broker using one of these messaging technologies. ZeroMQ (zmq) is the default core type and most commonly used but there are also cores that use TCP and UDP networking protocols directly (forgoing ZMQ's guarantee of delivery and reconnection functions), IPC (uses Boost's interprocess communication for fast in-memory message-passing but only works if all federates are running on the same physical computer), and MPI (for use on HPC clusters where MPI is installed). See the [User Guide page on core types](../user-guide/advanced_topics/CoreTypes.md) for more details.

---

### `core_name` [HELICS-generated]

_Alternative names:_ `corename` | `coreName`

_API:_ helicsFederateInfoSetCoreName
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1FederateInfo.html#abb18967525d94a2a98c1d30be12fcedc)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetCoreName)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetCoreName-Tuple{HELICS.FederateInfo,%20String})

Only applicable for `ipc` and `test` core types; otherwise can be left undefined.

---

### `core_init_string` | `-i` [null]

_Alternative names:_ `coreinitstring` | `coreInitString`

_API:_ `helicsFederateInfoSetCoreInitString`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1FederateInfo.html#ac466e079d28d9bf2fabb065ba0372b70)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetCoreInitString)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetCoreInitString-Tuple{HELICS.FederateInfo,%20String})

A command-line-like string that specifies options for the core as it connects to the federation. These options are:

- `--broker=` | `broker_address=` | `brokeraddress`: IP address of broker
- `--brokerport=`: Port number on which the broker is communicating
- `--broker_rank=`: For MPI cores only; identifies the MPI rank of the broker
- `--broker_tag=`: For MPI cores only; identifies the MPI tag of the broker
- `--localport=`: Port number to use when communicating with this core
- `--autobroker`: When included the core will automatically generate a broker (does not work for all core types)
- `--key=`: Specifies a key to use when communicating with the broker. Only federates with this key specified will be able to talk to the broker with the same `key` value. This is used to prevent federations running on the same hardware from accidentally interfering with each other.
- `--profiler=log` - Send the profiling messages to the default logging file. `log` can be replaced with a path to an alternative file where only the profiling messages will be sent. See the [User Guide page on profiling](../user-guide/advanced_topics/profiling.md) for further details. If a file is specified it is cleared.
- `--profiler_append=somefile.txt` - Send the profiling messages to file and leave the existing contents appending new data. See the [User Guide page on profiling](../user-guide/advanced_topics/profiling.md) for further details.

In addition to these options, all options shown in the `broker_init_string` are also valid.

---

### `autobroker` [false]

_API:_ (none)

Automatically generate a broker if one cannot be connected to. For federations with only one broker (often the case) and/or with federations containing custom federates that were developed for this particular application, it can be convenient to create the broker in the process of creating a specific federate; this option allows that to take place. The downside to this is it creates a federation with a small amount of mystery as the broker is not clearly shown to be launched as its own federate alongside the other federates and those unfamiliar with the federation composition may have to spend some extra time to understand where the broker is coming from. This option does not work for all core types, it is specifically designed for inproc and testing cores. Others may be added later.

---

### `broker_init_string` [null]

_Alternative names:_ `brokerinitstring`, `brokerInitString`

_API:_ `helicsFederateInfoSetBrokerInitString`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1FederateInfo.html#a167ad88f2315f46d6edcf920325b8ad0)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetBrokerInitString)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetBrokerInitString-Tuple{HELICS.FederateInfo,%20String})

String used to define the configuration of the broker if one is autogenerated. Such configuration typically includes things like broker IP addresses and port numbers. Again, if this is a co-simulation running on a single computer (and is the only HELICS co-simulation running on said computer) the default option is likely to be sufficient. The following options are available for this string:

- `--federates=` - The minimum number of federates expected to join a federation through the broker. Equivalent to `-f`.
- `--max_federates=` - The maximum number of federates allowed to join through a broker.
- `--name=` - Name of the broker; can be used by federates to specify which broker to use
- `--max_iterations=` - Maximum iterations allowed when using the re-iteration API
- `--min_broker_count=` - The minimum number of brokers that the co-simulation must have to begin initialization. (This option is not available for cores)
- `--max_broker_count=` - The maximum number of brokers that the co-simulation should allow. (This option is not available for cores)
- `--slow_responding` - Removes the requirement for the broker to respond to pings from other entities in the co-simulation in a timely manner and forces the assumption that this broker is still connected to the federation.
- `--restrictive_time_policy` - Forces the broker to use the most restrictive (conservative) timing policy when granting times to federates. Has the potential to increase co-simulation time as time grants may happen later then they actually need to.
- `--terminate_on_error` - All errors from any member of the federation will cause the broker to terminate the co-simulation for the entire federation.
- `--force_logging_flush` - Force writing to the log after every message.
- `--log_file=` - Name of file use for logging for this broker.
- `--log_level=` - Specifies the level of logging (both file and console) for this broker.
- `--file_log_level=` - Specifies the level of logging to file for this broker.
- `--console_log_level=` - Specifies the level of logging to file for this broker.
- `--dumplog` - Captures a record of all logging messages and writes them out to file or console when the broker terminates.
- `--globaltime` - Specify that the broker should use a globalTime coordinator to coordinate a master clock time with all federates.
- `--asynctime` - Specify that the federation should use the asynchronous time coordinator (only minimal time management is handled in HELICS and federates are allowed to operate independently).
- `--timing = ("async"|"global"|"default"|"distributed")` - specify the timing mode to use for time coordination.
  - `distributed` - Time management is distributed and managed by each federates. This is the default
  - `global` - HELICS centrally manages the time coordination; for larger federations this is likely to be slower
  - `async` - federates manage their own time with minimal coordination from HELICS (such as when using real-time simulators)
- `--tick=` - Heartbeat period in ms. When brokers fail to respond after 2 ticks secondary actions are taking to confirm the broker is still connected to the federation. Times can also be entered as strings such as "15s" or "75ms".
- `--global_disconnect` - Specify that federates should wait to disconnect until the federation is completed
- `--timeout=` milliseconds to wait for all the federates to connect to the broker (can also be entered as a time like '10s' or '45ms')
- `--network_timeout=` - Time to establish a socket connection in ms. Times can also be entered as strings such as "15s" or "75ms".
- `--error_timeout=` - Time in ms to wait after an error state is reached before terminating. Times can also be entered as strings such as "15s" or "75ms".
- `--query_timeout=` - Time in ms to wait for a query to complete. Times can also be entered as strings such as "15s" or "75ms".
- `--grant_timeout=` - Time in ms to wait to allow a time request to wait before triggering diagnostic actions. Times can also be entered as strings such as "15s" or "75ms".
- `--max_cosim_duration=`- The time in ms the co-simulation should be allowed to run. If the time is exceeded the co-simulation will terminate automatically.
- `--children=` - The minimum number of child objects the broker should expect before allowing entry to the initializing state.
- `--subbrokers=` - The minimum number of child objects the broker should expect before allowing entry to the initializing state. Same as `--children` but might be clearer in some cases with multilevel hierarchies.
- `--brokerkey=` - A broker key to use for connections to ensure federates are connecting with a specific broker and only appropriate federates connect with the broker. See [simultaneous co-simulations](../user-guide/advanced_topics/simultaneous_cosimulations.md) for more information.
- `--profiler=log` - Send the profiling messages to the default logging file. `log` can be replaced with a path to an alternative file where only the profiling messages will be sent. See the [User Guide page on profiling](../user-guide/advanced_topics/profiling.md) for further details. If a file is specified it is cleared.
- `--profiler_append=somefile.txt` - Send the profiling messages to file and leave the existing contents appending new data. See the [User Guide page on profiling](../user-guide/advanced_topics/profiling.md) for further details.
- `--time_monitor=` - Specify the name of the federate to monitor the time from and generate periodic log messages in the broker as the federate updates its time.
- `--time_monitor_period=` - can only be used with `--time_monitor`, set the minimum time period which must elapse in simulation before another log message from the time monitor is generated
- `--disable_timer` - Disables all timeout in broker operation
- `--debugging` - Equivalent to `--slow_responding --disable_timer`
- `--logbuffer` - Enable buffering recent log messages for retrieval with the "logs" query. Optionally specify the size of the circular log buffer; defaults to 10 messages if no size is supplied.
- `--allow_remote_control` - Enables the broker to respond to certain remote commands such as "disconnect"
- `--dynamic` - Allow for dynamic federations where federates can join after the co-simulation has begun. This capability is disabled by default.
- `--disable_dynamic_sources` - Prevents data sources from registering after the federation has entered initializing mode. This capability is **enabled** by default

The configuration options for a broker may now be fully specified in json or toml files instead of having to pass them as command line arguments.

### `terminate_on_error` [false]

_Alternative names:_ `terminateonerror`, `terminateOnError`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_TERMINATE_ON_ERROR` [72]

If the `terminate_on_error` flag is set then a federate encountering an internal error will trigger a global error and cause the entire federation to terminate. Errors of this nature are typically the result of configuration errors, such as having a required publication that is not used or incompatible units or types on publications and subscriptions. This is the same option that is available in the `broker_init_string` but applied only to a specific federate (whereas when applied at the broker level it is effectively applied to all federates).

---

### `source_only` | [false]

_Alternative names:_ `sourceonly`, `sourceOnly`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_SOURCE_ONLY` [4]

Used to indicate to the federation that this federate is only producing data and has no inputs/subscriptions. Specifying this when appropriate allows HELICS to more efficiently grant times to the federation.

---

### `observer` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_OBSERVER` [0]

Used to indicate to the federation that this federate produces no data and only has inputs/subscriptions. Specifying this when appropriate allows HELICS to more efficiently grant times to the federation.

---

### `reentrant` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_REENTRANT` [38]

Used to indicate to the broker that this federate may disconnect and reconnect at a later time using the same federate name. Without setting this flag, the federate would have to rejoin under a different name and would be considered a new federate by the federation. This flag only has an effect if the "dynamic" flag is also set on the broker.

---

### `broker_key` [null]

_API:_ `helicsFederateSetBrokerKey`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1BrokerBase.html#a45c3f109160b1054f46a6734da6f1557)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetBrokerKey)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetBrokerKey-Tuple{HELICS.FederateInfo,%20String})

Setting a broker key only allows federates that have the same broker key to be part of the federation.

## Logging Options

### `log_file` [null]

_Alternative names:_ `logfile`, `logFile`
_API:_ `helicsFederateSetLogFile`

[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Logger.html)
| [C](api-reference/C_API.md#federate)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateSetLogFile)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsBrokerSetLogFile-Tuple{HELICS.Broker,%20String})

Specifies the name of the log file where logging messages will be written.

---

### `log_level` [0]

_Alternative names:_ `loglevel`, `logLevel`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_LOG_LEVEL` [271]

_Valid values:_

- `none` - `HELICS_LOG_LEVEL_NO_PRINT`
- `no_print` - `HELICS_LOG_LEVEL_NO_PRINT`
- `error` - `HELICS_LOG_LEVEL_ERROR`
- `profiling` - `HELICS_LOG_LEVEL_PROFILING`
- `warning` - `HELICS_LOG_LEVEL_WARNING`
- `summary` - `HELICS_LOG_LEVEL_SUMMARY`
- `connections` - `HELICS_LOG_LEVEL_CONNECTIONS`
- `interfaces` - `HELICS_LOG_LEVEL_INTERFACES`
- `timing` - `HELICS_LOG_LEVEL_TIMING`
- `data` - `HELICS_LOG_LEVEL_DATA`
- `debug` - `HELICS_LOG_LEVEL_DEBUG`
- `trace` - `HELICS_LOG_LEVEL_TRACE`

Determines the level of detail for log messages. In the list above, the keywords on the left can be used when specifying the logging level via a JSON configuration file. The enumerations on the right are used when configuring via the API.

---

### `file_log_level` [null]

_Alternative names:_ `fileloglevel`, `fileLogLevel`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_FILE_LOG_LEVEL` [272]

_Valid values:_ Same as in `loglevel`

Allows a distinct log level for the written log file to be specified. By default the logging level to file and console are identical and will only differ if `file_log_level` or `console_log_level` are defined.

---

### `console_log_level` [null]

_Alternative names:_ `consoleloglevel`, `consoleLogLevel`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL` [274]

_Valid values:_ Same as in `loglevel`

Allows a distinct log level for the written log file to be specified. By default the logging level to file and console are identical and will only differ if `file_log_level` or `console_log_level` are defined.

---

### `force_logging_flush` [false]

_Alternative names:_ `forceloggingflush`, `forceLoggingFlush`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_FORCE_LOGGING_FLUSH` [88]

Setting this option forces HELICS logging messages to be flushed to file after each one is written. This prevents the buffered IO most OSs implement to be bypassed such that all messages appear in the log file immediately after being written at the cost of slower simulation times due to more time spent writing to file.

---

### `dump_log` [false]

_Alternative names:_ `dumplog`, `dumpLog`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_DUMPLOG` [89]

When set, a record of all messages is captured and written out to the log file at the conclusion of the co-simulation.

---

### `logbuffer` [10]

_Alternative names:_ `log_buffer`, `logBuffer`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_LOG_BUFFER` [276]

When set to a number greater than 0 will enable the most recent X log messages of the object to be buffered for retrieval via the ["logs" query](../user-guide/advanced_topics/queries.md). Also see discussion in [Logging](../user-guide/fundamental_topics/logging.md#log-buffer).

## Other Options

### `indexgroup` [0]

_Alternative names:_ `index_group`, `indexGroup`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_INDEX_GROUP` [282]

When set to a number greater than 0 will modify the internal federateId codes. Values allowed are from 0 to 16. Each group allows 100,000,000 federates. In a few select situations, such as ordering messages, or breaking a deadlock in a timing situation, the ordering ties are broken by federate index. It is possible in some scenarios that this ordering can be variable from run to run introducing a source of randomness since federate id's are assigned in the order processed. The `indexGroup` parameter allows the user to directly influence the internal id, which can eliminate a source of randomness in a few select situations. Specifically, each increment of the index group increases the internal id by 100,000,000 from what it would have been. This allows 17 total allowable option group values 0-16. In most cases, this option will have no observable impact on co-simulation results. If exceeding 100,000,000 care must be exercised in its use to ensure federate Id's do not conflict, by using only larger intervals of increments.

## Timing Options

### `ignore_time_mismatch` [false]

_Alternative names:_ `ignoretimemismatch`, `ignoreTimeMismatch`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS` [67]

If certain timing options (_i.e._ `period`, or `minTimeDelta`) are used it is possible for the time granted a federate to be greater than the requested time. This situation would normally generate a warning message, but if this flag is set those warnings are silenced.

---

### `uninterruptible` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_UNINTERRUPTIBLE` [1]

Normally, a federate will be granted a time earlier than it requested when it receives a message from another federate; the presence of any message implies there could be an action the federate needs to take and may generate new messages of its own. There are times, though, when it is important that the federate only be granted a time (and begin simulating/executing again) that it has previously requested. For example, there could be some controller that should only operate at fixed intervals even if new data arrives earlier. In these cases, setting the `uninterruptible` flag will prevent premature time grants.

---

### `period` [1ns]

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_PROPERTY_TIME_PERIOD` [140]

Many time-based simulators have a minimum time-resolution or a user-configurable step size. The `period` parameter can be used to effectively synchronize the times that are granted with the defined simulation period. The default units for `period` are in seconds but the string for this parameter can include its own units (e.g. "2 ms" or "1 hour"). Setting `period` will force all time grants to occur at times of `n*period` even if subscriptions are updated, messages arrive, or the federate requests a time between periods. This value effectively makes the federates `uninterruptible` during the times between periods. Relatedly...

---

### `offset` [0]

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_PROPERTY_TIME_OFFSET` [141]

There may be cases where it is preferable to have a simulator receive time grants that are offset slightly in time to one or more other federates. Defining an `offset` value allows this to take place; units are handled the same as in `period`. Setting both `period` and `offset`, will result in the all times granted to the federate in question being constrained to `n*period + offset`.

---

### `time_delta` [1ns]

_Alternative names:_ `timeDelta`, `timedelta`

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_PROPERTY_TIME_DELTA` [137]

timeDelta has some similarities to `period`; where `period` constrained the granted time to regular intervals, `timeDelta` constrains the grant time to a minimum amount from the last granted time. Units are handled the same as in `period`.

---

### `input_delay` [0]

_Alternative names:_ `inputdelay`, `inputDelay`

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_PROPERTY_INPUT_TIME_DELAY` [148]

`inputDelay` specifies a delay in simulated time between when a signal arrives at a federate and when that federate is notified that a new value is available. `outputDelay` is similar but applies to signals being sent by a federate. Note that this applies to both value signals and message signals.

---

### `output_delay` [0]

_Alternative names:_ `outputdelay`, `outputDelay`

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_TIME_PROPERTY_OUTPUT_TIME_DELAY` [150]

`outputDelay` is similar to `input_delay` but applies to signals being sent by a federate. Note that this applies to both value signals and message signals.

---

### `real_time` [false]

_Alternative names:_ `realtime`, `realTime`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,Union{Int64,%20HELICS.Lib.helics_federate_flags},Bool})

_Property's enumerated name:_ `HELICS_FLAG_REALTIME` [16]

If set to true the federate uses `rt_lag` and `rt_lead` to match the time grants of a federate to the computer wall clock.
If the federate is running faster than real time this will insert additional delays. If the federate is running slower than real time this will cause a force grant, which can lead to non-deterministic behavior. `rt_lag` can be set to maxVal to disable force grant

---

### `rt_lag and `rt_lead`[0.2]

_Alternative names:_ `rtlag`, `rtLag`; `rtlead`, `rtLead`

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_PROPERTY_TIME_RT_LAG` [143] and `HELICS_PROPERTY_TIME_RT_LEAD` [144]

Defines "real-time" for HELICS by setting tolerances for HELICS to use when running a real-time co-simulation. HELICS is forced to keep simulated time within this window of wall-clock time. Most general purpose OSes do not provide guarantees of execution timing and thus very small values of `rt_lag` and `rt_lead` (less than 0.005) are not likely to be achievable.

---

### `rt_tolerance` [0.2]

_Alternative names:_ `rttolerance`, `rtTolerance`

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_PROPERTY_TIME_RT_TOLERANCE` [145]

Implements the same functionality of `rt_lag` and `rt_lead` but does so by using a single value to set symmetrical lead and lag constraints.

---

### `wait_for_current_time_update` [false]

_Alternative names:_ `waitforcurrenttimeupdate`, `waitForCurrentTimeUpdate`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE` [10]

If set to true, a federate will not be granted the requested time until all other federates have completed at least 1 iteration of the current time or have moved past it. If it is known that 1 federate depends on others in a non-cyclic fashion, this can be used to optimize the order of execution without iterating.

---

### `restrictive_time_policy` [false]

_Alternative names:_ `restrictivetimepolicy` | `restrictiveTimePolicy`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_RESTRICTIVE_TIME_POLICY` [11]

If set, a federate will not be granted the requested time until all other federates have completed at least 1 iteration of the current time or have moved past it. If it is known that 1 federate depends on others in a non-cyclic fashion, this can be used to optimize the order of execution without iterating.

Using the option `restrictive-time-policy` forces HELICS to use a fully conservative mode in granting time. This can be useful in situations beyond the current reach of the distributed time algorithms. It is generally used in cases where it is known that some federate is executing and will trigger someone else, but most federates won't know who that might be. This prevents extra messages from being sent and a potential for time skips. It is not needed if some federates are periodic and execute every time step. The flag can be used for federates, brokers, and cores to force very conservative timing with the potential loss of performance as well.

Only applicable to Named Input interfaces ([see section on value federate interface types](../user-guide/fundamental_topics/value_federates.md)), if enabled this flag checks that data type of the incoming signals match that specified for the input.

---

### `slow_responding` [false]

_Alternative names:_ `slowresponsing`, `slowResponding`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_SLOW_RESPONDING` [29]

If specified on a federate, setting this flag indicates the federate may be slow in responding, and to not forcibly eject the federate from the federation for the slow response. This is an uncommon scenario.

If applied to a core or broker (`--slow_responding` in the `core_init_string` or `broker_init_string`, respectively), it is indicative that the broker doesn't respond to internal pings quickly and should not be disconnected from the federation for the slow response.

---

### `event_triggered` [false]

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_FLAG_EVENT_TRIGGERED` [81]

For federates that are event-driven rather than timing driven, this flag must be set (to increase timing efficiency and avoid timing lock-ups). Event-driven federates are those that don't progress through simulation time at regular timesteps but instead wait for arriving messages to act. The most common examples are controller federates which generally request infinite time (well, `HELICS_TIME_MAXTIME`) and rely on HELICS to grant them an earlier time whenever a signal (often message) has arrived. Filter federates are another common federate type that must have this flag set.

## Iteration

### `forward_compute` [false]

_Alternative names:_ `forwardcompute` | `forwardCompute`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_FORWARD_COMPUTE` [14]

Indicates to the broker and the rest of the federation that this federate computes ahead of its granted time and can/does roll back when necessary. Federates able to do this (and who set this flag) allow more efficient time grants to the federation as a whole.

---

### `rollback` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_ROLLBACK` [12]

Indicates to the broker and the rest of the federation that this federate can/does roll back when necessary. Federates able to do this (and who set this flag) allow more efficient time grants to the federation as a whole.

---

### `max_iterations` [50]

_Alternative names:_ `maxiterations`, `maxIteration`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_MAX_ITERATIONS` [259]

For federates engaged in iteration (recomputing values based on updated inputs at a single simulation timestep) there may be a need to enforce a maximum number of iterations. This option allows that value to be set. When any federate reaches this number of iterations, HELICS will evaluate the federation as a whole and grant the next smallest time supported by the iterating federates. This time will only be granted to the federates that would be able to execute at this time.

## General and Per Subscription, Input, or Publication

These options can be set globally for all subscriptions, inputs and publications for a given federate. Even after setting them globally, they can be included in the configuration for an individual subscription, input, or publication, over-riding the global setting.

### `only_update_on_change` and `only_transmit_on_change` [false]

_Alternative names:_ `onlyupdateonchange`, `onlyUpdateOnChange`;`onlytransmitonchange`, `onlyTransmitOnChange`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_ONLY_UPDATE_ON_CHANGE` [454] and `HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE` [452]

Setting these flags prevents new value signals with the same value from being received by the federate or sent by the federate. Setting these flags will reduce the amount of traffic on the HELICS bus and can provide performance improvements in co-simulations with large numbers of messages.

---

### `tolerance` [0]

_API:_ `helicsPublicationSetMinimumChange` and `helicsInputSetMinimumChange`

[C++ input](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Input.html#a55056ac9dd2895270f575827dd9951c7) and [C++ publication](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Publication.html#ab66f5680bb4a5e062314f6f8e5dea846)
| [C input](api-reference/C_API.md#input) and [C publication](api-reference/C_API.md#publication)
| [Python input](https://python.helics.org/api/capi-py/#helicsInputSetMinimumChange) and [Python publication](https://python.helics.org/api/capi-py/#helicsPublicationSetMinimumChange)
| [Julia input](https://julia.helics.org/latest/api/#HELICS.helicsPublicationSetMinimumChange-Tuple{HELICS.Publication,%20Float64}) and [Julia publication](https://julia.helics.org/latest/api/#HELICS.helicsInputSetMinimumChange-Tuple{HELICS.Input,%20Float64})

This option allows the specific numerical definition of "change" when using the `only_update_on_change` and `only_transmit_on_change` options.

---

### `default` [0]

_API:_ `helicsInputSetDefaultX`
[C++ input](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Input.html#a55056ac9dd2895270f575827dd9951c7)
| [C input](api-reference/C_API.md#input)
| [Python input](https://python.helics.org/api/capi-py/#helicsInputSetDefault)
| [Julia input](https://julia.helics.org/latest/api/#HELICS.helicsInputSetDefaultBoolean-Tuple{HELICS.Input,%20Bool})

This option allows specifying the default value used when no publication has been received. Each datatype has its own API call such as:

- `helicsInputSetDefaultBoolean()`
- `helicsInputSetDefaultBytes()`
- `helicsInputSetDefaultChar()`
- `helicsInputSetDefaultComplex()`
- `helicsInputSetDefaultComplexVector()`
- `helicsInputSetDefaulDouble()`
- `helicsInputSetDefaultInteger()`
- `helicsInputSetDefaultRaw()`
- `helicsInputSetDefaultString()` (used for JSONs)
- `helicsInputSetDefaultVector()`

Though they are not as obviously named, the following two APIs do provide a means of setting the default value as well:

- `helicsInputSetDefaultTime()` - set the default using a HelicsTime value
- `helicsInputSetDefaultNamedPoint()` -set the default NamedPoint which is a pair of a string and double used for tagged values or set points

---

### `connection_required` [false]

_Alternative names:_ `connectionrequired` | `connectionRequired`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_CONNECTION_REQUIRED` [397]

When a federate is initialized, one of its tasks is to make sure the recipients of directed signals exist. If, after the federation is initialized, the recipient can't be found, then by default a warning is generated and written to the log file. If the `connections_required` flag is set, this warning becomes a fatal error that stops the co-simulation.

- `publications` - At least one federate must subscribe to the publications.
- `subscriptions` - The message being subscribed to must be provided by some other publisher in the federation.

---

### `connection_optional` [true]

_Alternative names:_ `connectionoptional`, `connectionOptional`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL` [402]
_Property's enumerated name:_ `HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL` [402]

When an interface requests a target it tries to find a match in the federation. If it cannot find a match at the time the federation is initialized, then the default is to generate a warning. This will not halt the federation but will display a log message. If the `connections_optional` flag is set on a federate all subsequent `addTarget` calls on any interface will not generate any message if the target is not available.

---

### `default_global` [false]

_Alternative names:_ `defaultglobal`, `defaultGlobal`

_API:_ (no API interface)

Set to `true` to force all handles to act as globals; see "global" below for further details.

---

## Subscription, Input, and/or Publication Options

These options are valid for subscriptions, inputs, and/or publications (generically called "handles"). The APIs for dealing with registering these handles combine multiple options in the JSON config file and have varying levels of specificity (defining the date type for the handle or defining the handle as global). Rather than listing all APIs for the following options, the main APIs will be listed here and those using them can consult the API references to see which specific APIs are most applicable.

`helicsFederateRegisterPublication`
[C++](https://docs.helics.org/en/latest/doxygen/ValueFederate_8h.html#ac00c4e17aeb9e20fdf0f42fb8bc63d29)
| [C](api-reference/C_API.md#federate)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterPublication)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterPublication)

`helicsFederateRegisterSubscription`
[C++](https://docs.helics.org/en/latest/doxygen/ValueFederate_8h.html#ab30020ca06ad37548691f313df42e15f)
| [C](api-reference/C_API.md#federate)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterSubscription)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterSubscription)

`helicsFederateRegisterInput`
[C++](https://docs.helics.org/en/latest/doxygen/ValueFederate_8h.html#aa41b313f5e527055444bf915c8da3258)
| [C](api-reference/C_API.md#federate)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterInput)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterInput)

### `key` (required)

- `publications` - The string in this field is the unique identifier (at the federate level) for the value that will be published to the federation. If `global` is set (see below) it must be unique to the entire federation.
- `subscriptions` - This string identifies the federation-unique value that this federate wishes to receive. Unless `global` has been set to `true` in the publishings JSON configuration file, the name of the value is formatted as `<federate name>/<publication key>`. Both of these strings can be found in the publishing federate's JSON configuration file as the `name` and `key` strings, respectively. If `global` is `true` the string is just the `key` value.
- `input` - The string in this field is the unique identifier (at the federate level) that defines the input to receive value signals.

---

### `type` [null]

HELICS supports data types and data type conversion ([as best it can](../developer-guide/typeConversions.md)).

---

### `unit` [null]

HELICS is able to do some levels of unit conversion, currently only on double type publications but more may be added in the future. The units can be any sort of unit string, a wide assortment is supported and can be compound units such as m/s^2 and the conversion will convert as long as things are convertible. The unit match is also checked for other types and an error if mismatching units are detected. A warning is also generated if the units are not understood and not matching. The unit checking and conversion is only active if both the publication and subscription specify units.

---

### `global` [false]

(publications only) `global` is used to indicate that the value in `key` will be used as a global name when other federates are subscribing to the message. This requires that the user ensure that the name is used only once across all federates. Setting `global` to `true` is handy for federations with a small number of federates and a small number of message exchanges as it allows the `key` string to be short and simple. For larger federations, it is likely to be easier to set the flag to `false` and accept the extra naming.

---

### `buffer_data` [false]

_Alternative names:_ `bufferdata` | `bufferData`

_API:_ `helicsInputSetOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Input,%20Union{Int64,%20HELICS.Lib.HelicsHandleOptions},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_BUFFER_DATA` [411]

(only valid for inputs and subscriptions) Setting this flag will buffer the last value sent during the initialization phase of HELICS co-simulations. When the execution phase begins, that value will be resent to the receiving handle.

---

### `ignore_units_mismatch` [null]

_Alternative names:_ `ignoreunitmismatch`, `ignoreUnitMismatch`

_API:_ TODO
Under normal operation, handles that are connected (value signals flowing between them) are required to have units that either match or can be directly converted between. If mismatching units are connected, an error is thrown; when this flag is set that error is suppressed.

---

### `info` [""]

_API:_ `helicsInputSetInfo`
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga207530cdd10afb89581baea6977e60b8)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetInfo)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetInfo-Tuple{HELICS.Input,%20String})
The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example.

---

### `strict_input_type_checking` [false]

_Alternative names:_ `strictinputtypechecking`, `strictInputTypeChecking`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING` [414]

Generally, HELICS does [data type conversions where supported](../developer-guide/typeConversions.md) on connected value handles. That is, if a publication is specified as an `int` and the subscription is specified as a `double` HELICS will convert the value behind the scenes. Some of these conversions, though, may not be expected; for example, how is HELICS going to convert a complex value to a double? To ensure that no surprises take place in the data type conversion, setting this flag tells HELICS to require the sending and receiving handles to match in datatype. If they do not, an error is thrown and the co-simulation halts.

---

## Publication-only Options

### `targets` [null]

[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Publication.html#abb36f297be67589f7a73c4c1fc39240b)
| [C](api-reference/C_API.md#publication)
| [Python](https://python.helics.org/api/capi-py/#helicsPublicationAddTarget)
| [Julia](https://julia.helics.org/latest/api/#Publication)

Used to specify which inputs should receive the values from this output. This can be a list of output keys/names.

## Input-only Options

Inputs can receive values from multiple sending handles and the means by which those multiple data points for a single handle are managed can be specified with several options. See the [User Guide entry](../user-guide/advanced_topics/multiSourceInputs.md) for further details.

### `targets`

[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Input.html#a017ad953343ac7758fbf1f45ea54d1eb)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputAddTarget)
| [Julia](https://julia.helics.org/latest/api/#Input)

Inputs can specify which outputs (typically publications) they should be pulling from. This is similar to subscriptions but inputs can allow multiple outputs to feed to the same input. This can be a list of output keys/names.

---

### `connections` [null]

_API:_ `helicsInputSetOption`
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga7eb9a9058a92cb27b6b370fbff5a5ceb)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Input,%20Union{Int64,%20HELICS.Lib.HelicsHandleOptions},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_CONNECTIONS` [522]

Allows an integer number of connections to be considered value for this input handle. Similar to `multiple_connections_allowed` but allows the number of sending handles to be defined to a specific number.

---

### `input_priority_location` [null]

_Alternative names:_ `inputprioritylocation`, `inputPriorityLocation`

_API:_ `helicsInputSetOption`
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga7eb9a9058a92cb27b6b370fbff5a5ceb)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Input,%20Union{Int64,%20HELICS.Lib.HelicsHandleOptions},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION` [510]

When receiving values from multiple sending handles, when the values are received they or organized as a vector. This option is used to define which value in that vector has priority. The API can be called multiple times to set successive priorities.

---

### `clear_priority_list` [false]

_Alternative names:_ `clearprioritylist`, `clearPriorityList`

_API:_ `helicsInputSetOption`
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga7eb9a9058a92cb27b6b370fbff5a5ceb)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Input,%20Union{Int64,%20HELICS.Lib.HelicsHandleOptions},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST` [512]

When receiving values from multiple sending handles, when the values are received they or organized as a vector. This option is used to clear that priority list and redefine which values have priority.

---

### `single_connection_only` [false]

_Alternative names:_ `singleconnectiononly`, `singleConnectionOnly`

_API:_ `helicsInputSetOption`
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga7eb9a9058a92cb27b6b370fbff5a5ceb)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Input,%20Union{Int64,%20HELICS.Lib.HelicsHandleOptions},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY` [407]
When set, this forces the input handle to have only one sending handle it will receive from. Setting this flag serves as a sort of double-check to ensure that only one other handle is sending to this input handle and that the federation has been configured as expected.

---

### `multiple_connections_allowed` [true]

_Alternative names:_ `multipleconnectionsallowed`, `multipleConnectionsAllowed`

_API:_ `helicsInputSetOption`
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga7eb9a9058a92cb27b6b370fbff5a5ceb)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Input,%20Union{Int64,%20HELICS.Lib.HelicsHandleOptions},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED` [409]

When set, this flag allows the input handle to receive valuesfrom multiple other handles.

---

### `multi_input_handling_method` [`none`]

_Alternative names:_ `multiinputhandlingmethod`, `multiInputHandlingMethod`

_API:_ `helicsInputSetOption`
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga7eb9a9058a92cb27b6b370fbff5a5ceb)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Input,%20Union{Int64,%20HELICS.Lib.HelicsHandleOptions},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD` [507]
_Property values:_

- `none` | `no_op`
- `or`
- `sum`
- `max`
- `min`
- `average`
- `mean`
- `vectorize`
- `diff`

Given that an input can have multiple data sources, a method of reducing those multiple values into one needs to be defined. HELICS supports a number of mathematical operation to perform this reduction.

---

## Endpoint Options

As in the value handles, the registration of endpoints is done through a single API that incorporates multiple options. And as in the value handles, there is a `global` API option to allow the name of the endpoint to be considered global to the federation.

_API:_ `helicsFederateRegisterEndpoint`
[C++](https://docs.helics.org/en/latest/doxygen/MessageFederate_8h.html#a2eb3f4680791a817b7654f0b6ca97d4d)
| [C](api-reference/C_API.md#federate)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterEndpoint)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterEndpoint-Tuple{HELICS.Federate,%20String,%20String})

### `name` (required)

The name of the endpoint as it will be known to the rest of the federation.

---

### `type` [null]

_API:_ (none)

HELICS supports data types and data type conversion ([as best it can](https://www.youtube.com/watch?v=mZOAn-3aATY)).

---

### `destination` [null]

_Alternative names:_ `target`

_API:_ `helicsEndpointSetDefaultDestination`
[C++](https://docs.helics.org/en/latest/doxygen/MessageFederate_8h.html#a14821937c957403bb1392c7df6790529)
| [C](api-reference/C_API.md#endpoint)
| [Python](https://python.helics.org/api/capi-py/#helicsEndpointSetDefaultDestination)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsEndpointSetDefaultDestination-Tuple{HELICS.Endpoint,%20String})

Defines the default destination for a message sent from this endpoint.

---

### `alias` [null]

_API:_ (none)

Creates a local alias for a handle that may have a long name.

---

### `subscriptions` [null]

_API:_ `helicsEndpointSubscribe`
[C++](https://docs.helics.org/en/latest/doxygen/MessageFederate_8h.html#a4a46f16a60b4d4c539b7f8d7113d7b54)
| [C](api-reference/C_API.md#endpoint)
| [Python](https://python.helics.org/api/capi-py/#helicsEndpointSubscribe)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsEndpointSubscribe-Tuple{HELICS.Endpoint,%20String})

---

### `filters` [null]

See section on Filter Options.

---

### `info` [""]

_API:_ `helicsEndpointSetInfo`
[C++](https://docs.helics.org/en/latest/doxygen/MessageFederate_8h.html#af935c7f5ba547a19d1164c6d62f4f79a)
| [C](api-reference/C_API.md#endpoint)
| [Python](https://python.helics.org/api/capi-py/#helicsEndpointSetInfo)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsEndpointSetInfo-Tuple{HELICS.Endpoint,%20String})
The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example.

## Filter Options

Filters are registered with the core or through the application API.
There are also Filter object that hide some of the API calls in a slightly nicer interface. Generally a filter will be associated with a specific endpoint as either a source filter or destination filter. Source filters can be chained, as in there can be more than one of them. At present there can only be a single non-cloning destination filter attached to an endpoint.

Non-cloning filters can modify the message in some ways, cloning filters just copy the message and may send it to multiple destinations. Cloning is not considered the "operation" of the filter and can be specified in parallel to another filter operation.

On creation, filters have a target endpoint and an optional name. Custom filters may have input and output types associated with them; this is used for chaining and automatic ordering of filters. Filters do not have to be defined on the same core as the source or destination endpoint on which they act; they can be defined anywhere in the federation and all appropriate messages will be automatically routed through them appropriately.

_API:_

`helicsFederateRegisterFilter`
([C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#ad73223d527cfb6ff9179b7beb3a092cb)
| [C](api-reference/C_API.md#federate)
| [Python](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterFilter-Tuple{HELICS.Federate,%20Union{Int64,%20HELICS.Lib.HelicsFilterTypes},%20String})
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterFilter-Tuple{HELICS.Federate,%20Union{Int64,%20HELICS.Lib.HelicsFilterTypes},%20String}))
to create/register the filter and then

`helicsFilterAddSourceTarget`
([C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#a40d2017f51dca63c1b034df70c35c655)
| [C](api-reference/C_API.md#filter)
| [Python](https://python.helics.org/api/capi-py/#helicsFilterAddSourceTarget)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFilterAddSourceTarget-Tuple{HELICS.Filter,%20String}))
or `helicsFilterAddDestinationTarget`
([C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#aa197abc9f9c07f9d8fbe39aef588965f)
| [C](api-reference/C_API.md#filter)
| [Python](https://python.helics.org/api/capi-py/#helicsFilterAddDestinationTarget)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFilterAddDestinationTarget-Tuple{HELICS.Filter,%20String}))
to associate it with a specific endpoint

### `name` [null]

_API:_ (none)

Name of the filter; must be unique to a federate.

---

### `source_targets` [null]

_Alternative names:_ `sourcetargets`, `sourceTargets`

_API:_ `helicsFilterAddSourceTarget`
[C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#a40d2017f51dca63c1b034df70c35c655)
| [C](https://docs.helics.org/en/latest/references/api-reference/C_API.html#filter)
| [Python](https://python.helics.org/api/capi-py/#helicsFilterAddSourceTarget)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFilterAddSourceTarget-Tuple{HELICS.Filter,%20String})

Defines the endpoint sending messages that the filter should act on. All messages coming from the specified endpoint will run through the filter first before being sent on to their specified destination.

---

### `destination_targets` [null]

_Alternative names:_ `destinationtargets`, `destinationtargets`

_API:_ `helicsFilterAddDestinationTarget`
[C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#aa197abc9f9c07f9d8fbe39aef588965f)
| [C](https://docs.helics.org/en/latest/references/api-reference/C_API.html#filter)
| [Python](https://python.helics.org/api/capi-py/#helicsFilterAddDestinationTarget)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFilterAddDestinationTarget-Tuple{HELICS.Filter,%20String})

Defines the endpoint receiving messages that the filter should act on. All messages going to the specified endpoint will run through the filter first.

---

### `info` [null]

_API:_ `helicsFilterSetInfo`
[C++](https://docs.helics.org/en/latest/doxygen/group__Clone.html#gaea3f69061b5ee71dac977fad59f62083)
| [C](api-reference/C_API.md#filter)
| [Python](https://python.helics.org/api/capi-py/#helicsFilterSetInfo)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFilterSetInfo-Tuple{HELICS.Filter,%20String})
The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example.

---

### `operation` [null]

_API:_ `helicsFederateRegisterFilter`
[C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#ad73223d527cfb6ff9179b7beb3a092cb)
| [C](api-reference/C_API.md#federate)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterFilter)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterFilter-Tuple{HELICS.Federate,%20Union{Int64,%20HELICS.Lib.HelicsFilterTypes},%20String})

Filters have a predefined set of operations they can perform. The following list defines the valid operations for filters. Most filters require additional specifications in properties data structure, an example of which is shown for each filter type.

#### `reroute`

This filter reroutes a message to a new destination. it also has an optional filtering mechanism that will only reroute if some patterns are matching. The patterns should be specified by "condition" in the set string the conditions are regular expression pattern matching strings.

Example `property` object:

```json
   "operation": "reroute",
    "properties": [
        {
            "name": "newdestination",
            "value": "endpoint name    "
        },
        {
            "name": "condition",
            "value": "regular expression string"
        }
    ]
```

#### `delay`

This filter will delay a message by a certain amount fo time.

Example `property` object:

```json
   "operation": "delay",
    "properties": {
        "name": "delay",
        "value": "76 ms",
    },
```

#### `random_delay` [null]

_Alternative names:_ `randomdelay`, `randomDelay`
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
  "operation": "randomdelay",
  "properties": [
    {
      "name": "distribution",
      "value": "normal"
    },
    {
      "name": "mean",
      "value": 0
    },
    {
      "name": "stdev",
      "value": 1
    }
  ]
```

#### `random_drop` | `randomdrop` | `randomDrop`

This filter will randomly drop a message, the drop probability is specified, and is modeled as a uniform distribution between zero and one.

```json
   "operation": "random_drop",
    "properties": {
        "name": "prob",
        "value": 0.5,
    },
```

#### `clone`

Unlike other filters, cloning is not considered an "operation" and is enabled by setting the "clone" flag. (Prior to version 3.6, it was required to also define the filter "operation" to "clone".)

The clone filter takes any message sent from endpoints listed in the `source_targets` object and sends a copy to endpoints in the `delivery` object. Note that both of these can be strings for a single endpoint or lists for multiple endpoints. In the example below, all messages originating from the endpoints named "source_endpoint_1_name" and "source_endpoint_2_name" will be copied and sent on to the endpoint named "ep_that_receives_cloned_messages".

```json
   "clone": true,
   "source_targets":[
     "source_endpoint_1_name",
     "source_endpoint_2_name"
   ],
   "delivery": "ep_that_receives_cloned_messages"
```

## Translator Options

Translators are used to bridge the gap between the value and message interfaces allowing publications to be sent to endpoints and endpoint messages to be sent to inputs as values. A translator functions as publication, input, and endpoint that other interfaces including filters can connect to. Further details can be found on the [documentation page covering translators](../user-guide/advanced_topics/translators).

_API:_ `helicsCoreRegisterTranslator`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html#a34a960d2259756912ad1dd6e70609416)
| [C]()
| [Python](https://python.helics.org/api/capi-py/#helicsCoreRegisterTranslator)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsCoreRegisterTranslator-Tuple{HELICS.Core,%20Int64,%20String})

or

_API:_ `helicsFederateRegisterGlobalTranslator`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html#a16f2ce4896c3f5e72d353b10584102a5)
| [C]()
| [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterGlobalTranslator)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterGlobalTranslator-Tuple{HELICS.Federate,%20Int64,%20String})

---

### `name` [null]

_API:_ (none, done as part of registering the translator)

Name of the filter; must be unique to a federate.

---

### `type` [null]

_API:_ (none, done as part of registering the translator)

Type of translator; determines the format of the data on the endpoint side of the translator. Must be one of the following: `HELICS_TRANSLATOR_TYPE_CUSTOM`, `HELICS_TRANSLATOR_TYPE_JSON`, or `HELICS_TRANSLATOR_TYPE_BINARY`.

---

### `source_targets` [null]

_Alternative names:_ `sourcetargets`, `sourceTargets`

_API:_ `helicsTranslatorAddPublicationTarget`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html)
| [C]()
| [Python](https://python.helics.org/api/capi-py/#helicsTranslatorAddPublicationTarget)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsTranslatorAddPublicationTarget-Tuple{HELICS.Translator,%20String})

or

_API:_ `helicsTranslatorAddSourceEndpoint`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html)
| [C]()
| [Python](https://python.helics.org/api/capi-py/#helicsTranslatorAddSourceEndpoint)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsTranslatorAddSourceEndpoint-Tuple{HELICS.Translator,%20String})

Connects the specified publication to the translator's input or adds the translator's endpoint as a destination for all messages coming from the specified endpoint.

---

### `destination_targets` [null]

_Alternative names:_ `destinationtargets`, `destinationTargets`

_API:_ `helicsTranslatorAddInputTarget`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html)
| [C]()
| [Python](https://python.helics.org/api/capi-py/#helicsTranslatorAddInputTarget)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsTranslatorAddInputTarget-Tuple{HELICS.Translator,%20String})

or

_API:_ `helicsTranslatorAddDestinationEndpoint`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html)
| [C]()
| [Python](https://python.helics.org/api/capi-py/#helicsTranslatorAddDestinationEndpoint)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsTranslatorAddDestinationEndpoint-Tuple{HELICS.Translator,%20String})

Connects the specified input to the translator's publication (output) or adds the specified endpoint as a destination for all messages coming from the translator's endpoint.

---

## Profiling

HELICS has a profiling capability that allows users to measure the time spent waiting for HELICS to grant it time and how much time is spent executing native code. These measurements are the foundation to understanding how to improve computation performance in a federation. Further details are provided in the [Profiling page in the User Guide.](../user-guide/advanced_topics/profiling.md) When enabling profiling at the federate level there are a few APIs that can be utilized.

### `profiling` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_PROFILING` [93]

Setting this flag enables profiling for the federate.

---

### `profiler` [null]

Turns on profiling for the federate and allows the specification of the log file where profiling messages will be written. No API is possible with this option as it must be specified prior to the creation of the federates.

---

### `local_profiling_capture` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_LOCAL_PROFILING_CAPTURE` [96]

Setting this flag sends the profiling messages to the local federate log rather than propagating them up to the core and/or broker.

---

### `profiling_marker` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_PROFILING_MARKER` [95]

Generates an additional `marker` message if logging is enabled.

## Network

For most HELICS users, most of the time, the following network options are not needed. They are most likely to be needed when working in complex networking environments, particularly when running co-simulations across multiple sites with differing network configurations. Many of these options require non-trivial knowledge of network operations and rather and it is assumed that those that needs these options will understand what they do, even with the minimal descriptions given.

### interface network

_API:_

See multiple options for --local, --ipv4, --ipv6, --all

---

### `reuse_address` [false]

_Alternative names:_ `reuseaddress`, `reuseAddress`

_API:_ (none)

Allows the server to reuse a bound address, mostly useful for tcp cores.

---

### `noack_connect` [false]

_Alternative names:_ `noackconnect`, `noackConnect`
Specify that a connection_ack message is not required to be connected with a broker.

---

### `max_size` [4096]

_Alternative names:_ `maxsize`, `maxSize`

_API:_ (none)

Message buffer size. Can be increased for large messages to limit the number of retries by the underlying networking protocols.

---

### `max_count` [256]

_Alternative names:_ `maxcount`, `maxCount`

_API:_ (none)

Maximum number of messages in queue. Can be increased for large volumes of messages to limit the number of retries by the underlying networking protocols.

---

### `network_retries` [5]

_Alternative names:_ `networkretries`, `networkRetries`

_API:_ (none)
Maximum number of network retry attempts.

---

### `encrypted` [false]

_API:_ (none)
set to true to enable encryption on network types that support encryption

---

### `encryption_config`

_Alternative names:_ `encryptionconfig`, `encryptionConfig`

_API:_ (none)
specify json or a file containing the configuration options for defining the encrypted interface

---

### `use_os_port` [false]

_Alternative names:_ `useosport`, `useOsPort`

_API:_ (none)
Setting this flag specifies that the OS should set the port for the HELICS message bus. HELICS will ask the operating system which port to use and will use the indicated port.

---

### `client` or `server` [null]

_API:_ (none)
specify that the network connection should be a server or client. By default neither option is enabled.

---

### `local_interface` [local IP address]

_Alternative names:_ `localinterface`, `localInterface`

_API:_ (none)
Specifies the IP address (and optionally port) the rest of the federation should use when contacting this federate.

---

### `broker_address` [local IP address]

_Alternative names:_ `brokeraddress`, `brokerAddress`

_API:_ (none)

Specifies the IP address (and optionally port) a federate or sub-broker should use when contacting its parent broker

---

### `broker_port` []

_Alternative names:_ `brokerport`, `brokerPort`

_API:_ (none)

Specifies the port a federate or sub-broker should use when contacting its parent broker

---

### `broker_name` [null]

_Alternative names:_ `brokername`, `brokerName`

_API:_ (none)

---

### `local_port` []

_Alternative names:_ `localport`, `localPort`
_API:_ (none)

Specifies the port the rest of the federation should use when contacting this federate.

---

### `port_start` []

_Alternative names:_ `portstart`, `portStart`

_API:_ (none)
starting port for automatic port definitions.

---

### `force` [false]

_API:_ (none)
Flag specifying that the broker network connection should attempt to override and terminate any existing broker using the specified port.
