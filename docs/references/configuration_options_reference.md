# Configuration Options Reference

```{eval-rst}
.. toctree::
    :hidden:
    :maxdepth: 2
```

Many of the HELICS entities have significant configuration options. Rather than comprehensively list these options while explaining the features themselves, we've created this section of the User Guide to serve as a reference as to what they are, what they do, and how to use them. This reference is oriented-around the use of JSON files for configuration and is an attempt to be comprehensive in listing and explaining those options. As will be explained below, many of these options are accessible via direct API calls though some of these calls are general in nature (such as [helicsFederateInfoSetIntegerProperty](api-reference/C_API.md#federateinfo) to set the logging level, among other things).

## Configuration methods

Generally, there are three ways in which a co-simulation can be configured and all the various options can be defined:

1. Using direct API calls in the federate source code.
2. Using command-line switches/flags while beginning execution of the federate
3. Using a JSON or TOML configuration file with `helicsCreateValueFederateFromConfig`, `helicsCreateMessageFederateFromConfig`, or `helicsCreateCombinationFederateFromConfig`.

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

Use federate settings and interface definitions with the federate creation APIs above. In a federate file, use `core` and `broker` sections to configure a new core and its broker connection. You can also use `core_init_string` for core options and `broker_init_string` for options for a broker created by `autobroker`. An app that accepts federate options may also have its own settings. See the [application references](apps/index.md).

The command-line parser generally ignores case and underscores in option names. For example, `--core_type` and `--coretype` are equivalent. Interface keys such as `defaultglobal` and `inputType` are read directly from the file, so use the spellings shown in this reference. Boolean federate settings without a dedicated switch can be set with `--flags`, as described below.

For federate timing properties in JSON or TOML, a bare number is in seconds. On the command line and in broker or core initialization options, a bare timing value is in milliseconds. Use explicit units such as `"100 ms"` or `"2 s"` when sharing settings between formats. A federate timing property also accepts a JSON object or TOML inline table with `value` and `unit` (or `units`).

Defaults, where specified, appear in brackets. The enumerated names shown for API settings can be passed to the corresponding C API setters or their language binding equivalents.

## Sample Configurations

The following JSON file configures a combination federate with both value and message interfaces. Interface names are local by default, so the input's target includes the federate name.

```json
{
  "name": "plant",
  "core_type": "inproc",
  "core": {"logfile": "plant.log"},
  "broker": {"auto": true, "initstring": "--federates=1"},
  "period": 1,
  "grant_timeout": "10 s",
  "publications": [
    {"key": "temperature", "type": "double", "unit": "degC"}
  ],
  "inputs": [
    {"key": "reading", "type": "double", "unit": "degC", "targets": ["plant/temperature"]}
  ],
  "endpoints": [
    {"name": "events", "tags": {"role": "monitoring"}}
  ],
  "datasinks": [
    {"name": "temperature_sink", "subscriptions": ["plant/temperature"]}
  ]
}
```

The equivalent TOML representation uses arrays of tables for interfaces.

```toml
name = "plant"
core_type = "inproc"
period = 1
grant_timeout = "10 s"

[core]
logfile = "plant.log"

[broker]
auto = true
initstring = "--federates=1"

[[publications]]
key = "temperature"
type = "double"
unit = "degC"

[[inputs]]
key = "reading"
type = "double"
unit = "degC"
targets = ["plant/temperature"]

[[endpoints]]
name = "events"
tags = {role = "monitoring"}

[[datasinks]]
name = "temperature_sink"
subscriptions = ["plant/temperature"]
```

## General Federate Options

These options control federate creation and behavior.

### `name` | `-n` [HELICS-generated]

The federate name must be unique within the federation. HELICS generates a name if the creation call and configuration supply none. The name is also the prefix for local interfaces. For example, a publication named `temperature` on a federate named `plant` can be targeted as `plant/temperature`.

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

Names the core to create or find. Federates in the same process can specify the same core name to share an existing core.

---

### `core` and `force_new_core`

`core` accepts a core type or the name of an existing core. Use `core_type` to require a recognized type and `core_name` to specify the name separately. Set `force_new_core` to `true` to have the federate create a core instead of reusing one. Its default is `false`.

### `separator` [`/`]

Single character between a federate name and its local interface names. Use the same resulting name in other interfaces' targets.

### `core_init_string` | `-i` [null]

_Alternative names:_ `coreinitstring` | `coreInitString`

_API:_ `helicsFederateInfoSetCoreInitString`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1FederateInfo.html#ac466e079d28d9bf2fabb065ba0372b70)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetCoreInitString)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetCoreInitString-Tuple{HELICS.FederateInfo,%20String})

A command-line-like string that specifies options for the core as it connects to the federation. These options are:

- `--broker` accepts a name or network address. Use `--broker_address` to specify an address explicitly.
- `--brokerport=`: Port number on which the broker is communicating
- `--broker_rank=`: For MPI cores only; identifies the MPI rank of the broker
- `--broker_tag=`: For MPI cores only; identifies the MPI tag of the broker
- `--localport=`: Port number to use when communicating with this core
- `--autobroker`: When included the core will automatically generate a broker (does not work for all core types)
- `--broker_key` sets the key for broker connections. The core and broker must use the same key.
- `--profiler=log` - Send the profiling messages to the default logging file. `log` can be replaced with a path to an alternative file where only the profiling messages will be sent. See the [User Guide page on profiling](../user-guide/advanced_topics/profiling.md) for further details. If a file is specified it is cleared.
- `--profiler_append=somefile.txt` - Send the profiling messages to file and leave the existing contents appending new data. See the [User Guide page on profiling](../user-guide/advanced_topics/profiling.md) for further details.

See [Core and Broker Options](#core-and-broker-options) for shared options and [Network](#network) for transport settings.

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

Initialization options for the broker created by `autobroker`. See [Core and Broker Options](#core-and-broker-options) and [Network](#network).

---

### `broker` section

The federate accepts `broker` or `broker_address` as a string. To group broker settings, use a `"broker"` object in JSON or a `[broker]` table in TOML. Its keys are `host` or `name` for the parent broker, `port` for its port, `auto` for automatic creation, `key` for the broker key, and `initstring` for the autogenerated broker's options. The federate CLI exposes the same keys as options under its `broker` subcommand.

### `flags` | `--flag` | `-f`

On the federate command line, pass a comma-separated list of names, for example `--flags=source_only,-strict_config_checking`. A leading minus disables a named flag. In a JSON or TOML federate file, prefer explicit keys with Boolean values. Interface flag lists use the syntax in [Interface metadata and options](#interface-metadata-and-options). If the application exposes the federate parser, use `--config` or `--config-file` to load settings from an INI, JSON, or TOML file.

| Federate setting | Default | Effect |
| --- | --- | --- |
| `strict_config_checking` | `true` | Report invalid filter and translator definitions as errors. Set to `false` to log warnings and skip those definitions. |
| `json` | `false` | Use JSON serialization for core communications. The named flag `use_json_serialization` is also recognized. Set `json` when configuring a new core from the federate. |
| `debugging` | `false` | Pass debugging mode to the newly created core. See the core and broker timeout options below. |
| `allow_remote_control`, `disable_remote_control` | `true`, `false` | Control whether the federate responds to remote operations such as disconnect. |
| `single_thread_federate` | `false` | Use single-threaded federate operation. Call the federate from a single thread when enabling this flag. |

The named flags `delay_init_entry` and `enable_init_entry` are core controls. Use the core API to hold and release initialization. The parser also recognizes `single_thread_core` and `multi_thread_core` as federate flag names. Core transport and threading are selected through the core type.

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

### `source_only` [false]

_Alternative name:_ `source`.

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

_API:_ `helicsFederateInfoSetBrokerKey`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1BrokerBase.html#a45c3f109160b1054f46a6734da6f1557)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetBrokerKey)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetBrokerKey-Tuple{HELICS.FederateInfo,%20String})

Setting a broker key only allows federates that have the same broker key to be part of the federation.

## Logging Options

### `log_file` [unset]

_Configuration:_ Core or broker initialization. For a federate file, use `core_init_string` to configure its new core, or use the federate API below.

_Alternative names:_ `logfile`, `logFile`
_API:_ `helicsFederateSetLogFile`

[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Logger.html)
| [C](api-reference/C_API.md#federate)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateSetLogFile)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsBrokerSetLogFile-Tuple{HELICS.Broker,%20String})

Specifies the name of the log file where logging messages will be written.

---

### `log_level` [`warning`]

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

### `file_log_level` [inherits `log_level`]

_Configuration:_ Core or broker initialization. For a federate file, use `core_init_string` to configure its new core.

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

### `console_log_level` [inherits `log_level`]

_Configuration:_ Core or broker initialization. For a federate file, use `core_init_string` to configure its new core.

_Alternative names:_ `consoleloglevel`, `consoleLogLevel`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL` [274]

_Valid values:_ Same as in `loglevel`

Sets the level for messages written to the console. It overrides the core or broker's general `log_level` for console output.

---

### `force_logging_flush` [false]

_Configuration:_ Core or broker initialization.

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

_Configuration:_ Core or broker initialization.

_Alternative names:_ `dumplog`, `dumpLog`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_DUMPLOG` [89]

When set, a record of all messages is captured and written out to the log file at the conclusion of the co-simulation.

---

### `logbuffer` [disabled]

_Configuration:_ Core or broker initialization. For a federate, use the integer property through the API below.

_Alternative names:_ `log_buffer`, `logBuffer`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_LOG_BUFFER` [276]

When set to a number greater than 0 will enable the most recent X log messages of the object to be buffered for retrieval via the ["logs" query](../user-guide/advanced_topics/queries.md). Also see discussion in [Logging](../user-guide/fundamental_topics/logging.md#log-buffer).

---

### `value_buffer_warning` [50]

_Alternative names:_ `valuebufferwarning`

_Command line:_ `--value_buffer_warning=<MiB>`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_VALUE_BUFFER_WARNING` [283]

Sets the warning and backpressure threshold for a receiving federate, in MiB. The threshold counts the total payload bytes in its queued future value updates across all inputs; it is not a per-input or federation-wide limit. The default is 50 MiB. Set this option to 0 to disable both the warning and buffer capacity notices. For example, `"value_buffer_warning": 20` in a federate configuration or `--value_buffer_warning=20` on its command line sets a 20 MiB threshold.

When the queued payload reaches the threshold, the receiver logs a warning and sends an advisory buffers-at-capacity control message to each source publisher for which it has no declared return publication or endpoint target. At twice the threshold it sends an urgent message. Each message carries the receiver's current granted time. The pressure level remains active until the queued payload falls below half the threshold, when the receiver sends a buffer-capacity-restored message. With the default setting, those levels are 50 MiB, 100 MiB, and below 25 MiB, respectively.

A publisher receiving an advisory notice holds its next time grant once its current granted time is at least eight steps ahead of the receiver's reported time. An urgent notice reduces that gap to four steps. A configured period defines the step; otherwise HELICS uses the time delta. When the time delta is only epsilon, HELICS uses the gap between the two most recent distinct grants, or the gap between the current grant and requested time if no earlier gap is available. The current grant and iterations at that time remain available. A restoration message releases the hold.

The threshold measures queued payload, not process memory. Metadata, other allocations, and values already in transit can make the process memory use substantially higher than this setting. The notice is sent when the receiver updates its queued-value count during time processing, so the setting is not a hard memory cap.

## Other Options

### `indexgroup` [0]

_Configuration:_ Federate integer property through the API. The property name lookup also recognizes the spellings below.

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

If certain timing options (for example, `period` or `time_delta`) are used it is possible for the time granted a federate to be greater than the requested time. This situation would normally generate a warning message, but if this flag is set those warnings are silenced.

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

The `interruptible` flag enables early grants and is the inverse of `uninterruptible`. By default a federate is interruptible.

### `period` [0]

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

_Property's enumerated name:_ `HELICS_PROPERTY_TIME_INPUT_DELAY` [148]

`inputDelay` specifies a delay in simulated time between when a signal arrives at a federate and when that federate is notified that a new value is available. `outputDelay` is similar but applies to signals being sent by a federate. Note that this applies to both value signals and message signals.

---

### `output_delay` [0]

_Alternative names:_ `outputdelay`, `outputDelay`

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_PROPERTY_TIME_OUTPUT_DELAY` [150]

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

### `rt_lag` and `rt_lead` [0]

_Alternative names:_ `rtlag`, `rtLag`; `rtlead`, `rtLead`

_API:_ `helicsFederateInfoSetTimeProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Core.html#a877611c1cfb0d30d1acb86b00f76eba3)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetTimeProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetTimeProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Union{Float64,%20Int64}})

_Property's enumerated name:_ `HELICS_PROPERTY_TIME_RT_LAG` [143] and `HELICS_PROPERTY_TIME_RT_LEAD` [144]

Defines "real-time" for HELICS by setting tolerances for HELICS to use when running a real-time co-simulation. HELICS is forced to keep simulated time within this window of wall-clock time. Most general purpose OSes do not provide guarantees of execution timing and thus very small values of `rt_lag` and `rt_lead` (less than 0.005) are not likely to be achievable.

---

### `rt_tolerance` [0]

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

Set this flag for a federate that waits for incoming events to determine when it advances. A callback federate in this mode requests the maximum simulation time by default and relies on incoming events to interrupt that request. The API is `helicsFederateInfoSetFlagOption` with `HELICS_FLAG_EVENT_TRIGGERED`.

### `stop_time` [unlimited]

_Alternative name:_ `stoptime`.

Set `stop_time` in a federate configuration file, pass `--stoptime` to the federate parser, or set `HELICS_PROPERTY_TIME_STOPTIME` through the federate info or federate API. A callback federate halts when a grant reaches or exceeds this simulation time. Federates that request time explicitly control when they stop in their simulation loops. See [Core and broker timeouts](#core-and-broker-timeouts) for the wall-clock federation limit.

### `grant_timeout` [0]

_Alternative name:_ `granttimeout`.

The wall-clock wait after a federate time request before HELICS begins diagnostic action for a missing grant. Zero disables this diagnostic timer. Use `helicsFederateInfoSetTimeProperty` with `HELICS_PROPERTY_TIME_GRANT_TIMEOUT`, or `--grant_timeout` on the federate command line. See [Timeouts](../user-guide/advanced_topics/timeouts.md).

## Iteration

### `forward_compute` [false]

_Alternative names:_ `forwardcompute` | `forwardCompute`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_FORWARD_COMPUTE` [14]

Reserved flag for a federate that computes ahead and can roll back. Current time coordination does not use it.

---

### `rollback` [false]

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_ROLLBACK` [12]

Reserved flag for a federate with rollback capability. Current time coordination does not use it.

---

### `max_iterations` [50]

_Alternative names:_ `maxiterations`, `maxIterations`

_API:_ `helicsFederateInfoSetIntegerProperty`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CommonCore.html#a01dddbf428e5d1f0ad8ead05491dabff)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetIntegerProperty)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetIntegerProperty-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsProperties},%20Int64})

_Property's enumerated name:_ `HELICS_PROPERTY_INT_MAX_ITERATIONS` [259]

For federates engaged in iteration (recomputing values based on updated inputs at a single simulation timestep) there may be a need to enforce a maximum number of iterations. This option allows that value to be set. When any federate reaches this number of iterations, HELICS will evaluate the federation as a whole and grant the next smallest time supported by the iterating federates. This time will only be granted to the federates that would be able to execute at this time.

## General and Per Subscription, Input, or Publication

Set `only_update_on_change`, `only_transmit_on_change`, `connection_required`, and `connection_optional` as federate-wide flags or on the applicable interfaces. Set `tolerance` and `default` on individual interfaces.

### `only_update_on_change` and `only_transmit_on_change` [false]

_Alternative names:_ `onlyupdateonchange`, `onlyUpdateOnChange`;`onlytransmitonchange`, `onlyTransmitOnChange`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_FLAG_ONLY_UPDATE_ON_CHANGE` [8] and `HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE` [6]. The corresponding interface options are `HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE` [454] and `HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE` [452].

Setting these flags prevents new value signals with the same value from being received by the federate or sent by the federate. Setting these flags will reduce the amount of traffic on the HELICS bus and can provide performance improvements in co-simulations with large numbers of messages.

---

### `tolerance` [unset]

_API:_ `helicsPublicationSetMinimumChange` and `helicsInputSetMinimumChange`

[C++ input](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Input.html#a55056ac9dd2895270f575827dd9951c7) and [C++ publication](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Publication.html#ab66f5680bb4a5e062314f6f8e5dea846)
| [C input](api-reference/C_API.md#input) and [C publication](api-reference/C_API.md#publication)
| [Python input](https://python.helics.org/api/capi-py/#helicsInputSetMinimumChange) and [Python publication](https://python.helics.org/api/capi-py/#helicsPublicationSetMinimumChange)
| [Julia input](https://julia.helics.org/latest/api/#HELICS.helicsPublicationSetMinimumChange-Tuple{HELICS.Publication,%20Float64}) and [Julia publication](https://julia.helics.org/latest/api/#HELICS.helicsInputSetMinimumChange-Tuple{HELICS.Input,%20Float64})

Set a positive threshold on a publication, input, or subscription to define the minimum numerical change to report. The loader ignores zero and negative values.

---

### `default` [type-dependent]

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
- `helicsInputSetDefaultDouble()`
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

### `connection_optional` [federate flag defaults to `false`]

_Alternative names:_ `connectionoptional`, `connectionOptional`

_API:_ `helicsFederateInfoSetFlagOption`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1CoreFederateInfo.html#a63efa7762fdc8a9d9869bbed6939448e)
| [C](api-reference/C_API.md#federateinfo)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateInfoSetFlagOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateInfoSetFlagOption-Tuple{HELICS.FederateInfo,%20Union{Int64,%20HELICS.Lib.HelicsFederateFlags},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL` [402]

When an interface requests a target it tries to find a match in the federation. If it cannot find a match at the time the federation is initialized, then the default is to generate a warning. This will not halt the federation but will display a log message. If the `connection_optional` flag is set on a federate all subsequent `addTarget` calls on any interface will not generate any message if the target is not available.

---

## Federate configuration sections

| Key | Meaning |
| --- | --- |
| `defaultglobal` | Default name visibility for publications, named inputs, endpoints, filters, and translators. Defaults to `false`. Set `global` on an individual interface to override it. |
| `interfaces` | In JSON, put `publications`, `subscriptions`, `inputs`, `endpoints`, `datasinks`, `filters`, and `translators` inside this object or at the same level as `name`. If `interfaces` is present, registration reads the lists inside it. In TOML use top-level arrays of tables. |
| `helics` | In JSON, group HELICS configuration in a `helics` object to use a file that also contains simulator settings. |
| `core` | When creating a core from a federate file, HELICS reads core initialization options from the JSON `core` object or TOML `[core]` table. A scalar `core` setting selects a core type or name. |
| `globals` | Set broker-visible global values. Use an object/table of string values, or an array of `[name, value]` pairs. |
| `aliases` | Add alternate names for registered interfaces. Use an object/table mapping original interface names to aliases, or an array of `[interface, alias]` pairs. This also works for endpoints, filters, and translators. |
| `potential_interfaces` | In JSON, declare interfaces for the [Connector](apps/Connector.md) to request during initialization. Supports `publications`, `inputs`, `endpoints`, `filters`, `translators`, and `datasinks`. |
| `potential_interface_templates` | In JSON, declare Connector templates such as `publication_templates`, `input_templates`, or `endpoint_templates`. The same template lists can appear inside `potential_interfaces`. See [Connector](apps/Connector.md) for the template syntax. |

### Interface metadata and options

Interface objects accept `info` for a descriptive string and `tags` for metadata. Federate-level `tags` attach metadata to the federate. The simplest JSON form is `"tags": {"role": "measurement"}`. JSON also accepts a `{"name": "role", "value": "measurement"}` record or an array of records, `[name, value]` pairs, or bare names with value `"1"`. In TOML use `tags = {role = "measurement"}` or an array of tables with `name` (or `key`) and `value`.

On interfaces, set supported Boolean handle options as keys such as `"connection_required": true` or in a `flags` list such as `"flags": ["connection_required", "-reconnectable"]`. A leading minus in the list clears that flag. The singular key `flag` takes one name. Use an integer or a documented name for options that take a value, such as `connections` or `multi_input_handling_method`.

`reconnectable` defaults to `false`. Enable it on an interface to allow the broker to reconnect it when a peer interface of the same name is recreated in a [dynamic federation](../user-guide/advanced_topics/dynamic_federations.md). The API option is `HELICS_HANDLE_OPTION_RECONNECTABLE`. The federate-level flag sets the default for its interfaces.

Target lists generally accept a string or an array under the plural key and a single string under the singular key. For keys with a source or destination prefix, the parser typically accepts snake_case, camelCase, and the spelling without underscores.

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

### `key` or `name`

- `publications` - The string in this field is the unique identifier (at the federate level) for the value that will be published to the federation. If `global` is set (see below) it must be unique to the entire federation.
- `subscriptions` - This string identifies the federation-unique value that this federate wishes to receive. Unless `global` has been set to `true` in the publishings JSON configuration file, the name of the value is formatted as `<federate name>/<publication key>`. Both of these strings can be found in the publishing federate's JSON configuration file as the `name` and `key` strings, respectively. If `global` is `true` the string is just the `key` value.
- `input` - The string in this field is the unique identifier (at the federate level) that defines the input to receive value signals.

---

### `type` [null]

HELICS supports data types and data type conversion ([as best it can](../developer-guide/typeConversions.md)).

---

### `unit` or `units` [unset]

HELICS is able to do some levels of unit conversion, currently only on double type publications but more may be added in the future. The units can be any sort of unit string, a wide assortment is supported and can be compound units such as m/s^2 and the conversion will convert as long as things are convertible. The unit match is also checked for other types and an error if mismatching units are detected. A warning is also generated if the units are not understood and not matching. The unit checking and conversion is only active if both the publication and subscription specify units.

---

### `global` [inherits `defaultglobal`]

Set on a publication or named input to make its key the federation-wide name. Local names use the federate prefix and separator. A subscription uses the target publication's name.

### `alias` [unset]

Adds an alternate name for a publication, input, or subscription. The federate-level `aliases` mapping also accepts aliases for other interface types.

### `time_restricted` [0 ms]

_Alternative names:_ `timerestricted`, `timeRestricted`.

Minimum simulation-time gap between accepted updates. For a publication, HELICS suppresses publishes within the gap. For an input or subscription, HELICS discards received updates that arrive too close in simulation time. Set a whole number of milliseconds. The API option is `HELICS_HANDLE_OPTION_TIME_RESTRICTED`.

### `buffer_data` [false]

_Alternative names:_ `bufferdata`, `bufferData`.

Set on a publication to retain the most recently published value. The API is `helicsPublicationSetOption` with `HELICS_HANDLE_OPTION_BUFFER_DATA`.

---

### `ignore_unit_mismatch` [false]

_Alternative names:_ `ignoreunitmismatch`, `ignoreUnitMismatch`

_API:_ `helicsInputSetOption` with `HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH`.

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

A publication also accepts `destination_inputs` and `destination_targets`. Both name input interfaces to connect to the publication. Set `connections` to require an exact number of receiving inputs, or `single_connection_only` to require one.

## Input-only Options

Inputs can receive values from multiple sending handles and the means by which those multiple data points for a single handle are managed can be specified with several options. See the [User Guide entry](../user-guide/advanced_topics/multiSourceInputs.md) for further details.

### `targets`

[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Input.html#a017ad953343ac7758fbf1f45ea54d1eb)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputAddTarget)
| [Julia](https://julia.helics.org/latest/api/#Input)

Inputs can specify which outputs (typically publications) they should be pulling from. This is similar to subscriptions but inputs can allow multiple outputs to feed to the same input. This can be a list of output keys/names.

---

An input or subscription also accepts `source_publications` and `source_targets`. Both name publications to connect to that input. Set `uninterruptible` on an individual input to keep its updates from interrupting a time request (API option `HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS`).

### `connections` [0]

_API:_ `helicsInputSetOption`
[C++](https://docs.helics.org/en/latest/doxygen/group__Information.html#ga7eb9a9058a92cb27b6b370fbff5a5ceb)
| [C](api-reference/C_API.md#input)
| [Python](https://python.helics.org/api/capi-py/#helicsInputSetOption)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsInputSetOption-Tuple{HELICS.Input,%20Union{Int64,%20HELICS.Lib.HelicsHandleOptions},%20Bool})

_Property's enumerated name:_ `HELICS_HANDLE_OPTION_CONNECTIONS` [522]

Sets the exact number of publications that must connect to this input. A value of zero leaves the connection count unrestricted.

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
- `and`
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

### `name` or `key`

Endpoint name. Set `global` to `true` to use this name throughout the federation, or to `false` to prefix it with the federate name. It inherits `defaultglobal` when omitted.

### `type` [unset]

Optional application-defined string describing the endpoint's message payload.

### `targeted` [false]

Register a targeted endpoint. It sends to its configured destinations. Use `destination_targets` or `destination_endpoints` to list them. A federate-level `targeted` key supplies the default for its endpoints, and each endpoint can override it.

### `datasinks`

The `datasinks` array registers receive-only, global interfaces. Each entry accepts `name` or `key` and the endpoint options for incoming messages, subscriptions, and metadata. A sink can collect messages and receive updates from publications via `subscriptions`.

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

### Endpoint targets and filters

| Key | Effect |
| --- | --- |
| `source_endpoints`, `source_targets` | Connect sending endpoints to this endpoint. |
| `destination_endpoints`, `destination_targets` | Connect this endpoint to receiving endpoints. |
| `subscriptions`, `source_inputs` | Subscribe the endpoint to publication updates, which it receives as messages. |
| `filters`, `source_filters` | Attach filters to messages sent from this endpoint. |
| `destination_filters`, `destFilters` | Attach filters to messages delivered to this endpoint. |

To give an endpoint another name, use the federate-level `aliases` mapping.

---

### `subscriptions` [null]

_API:_ `helicsEndpointSubscribe`
[C++](https://docs.helics.org/en/latest/doxygen/MessageFederate_8h.html#a4a46f16a60b4d4c539b7f8d7113d7b54)
| [C](api-reference/C_API.md#endpoint)
| [Python](https://python.helics.org/api/capi-py/#helicsEndpointSubscribe)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsEndpointSubscribe-Tuple{HELICS.Endpoint,%20String})

---

### `filters` [null]

See [Endpoint targets and filters](#endpoint-targets-and-filters) and [Filter Options](#filter-options).

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

_Alternative names:_ `destinationtargets`, `destinationTargets`

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

### Filter visibility, types, and properties

Set `global` to override `defaultglobal`. Use `inputType` and `outputType` to specify custom filter types. Built-in operations require these fields to be omitted.

`source_endpoints` and `destination_endpoints` are aliases for `source_targets` and `destination_targets`.

`properties` sets operation-specific parameters. Supply a record with `name` and `value` or an array of such records. In TOML, use an inline table or an array of tables. Use numbers for numeric parameters and strings for names, destinations, conditions, and unit-qualified delays.

### `operation` [`custom`]

_API:_ `helicsFederateRegisterFilter`
[C++](https://docs.helics.org/en/latest/doxygen/MessageFilters_8h.html#ad73223d527cfb6ff9179b7beb3a092cb)
| [C](api-reference/C_API.md#federate)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterFilter)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterFilter-Tuple{HELICS.Federate,%20Union{Int64,%20HELICS.Lib.HelicsFilterTypes},%20String})

The supported built-in operations are listed below. With `custom` or an omitted operation, the application can install its own filter operator. In JSON, an omitted operation with `cloning: true` selects the clone operation.

#### `reroute`

This filter reroutes a message to a new destination. it also has an optional filtering mechanism that will only reroute if some patterns are matching. The patterns should be specified by "condition" in the set string the conditions are regular expression pattern matching strings.

Example filter entry

```json
{
  "operation": "reroute",
  "properties": [
    {
      "name": "newdestination",
      "value": "destination_endpoint"
    },
    {
      "name": "condition",
      "value": "regular expression string"
    }
  ]
}
```

#### `delay`

Delays a message by the configured duration. A numeric `delay` is in seconds. A string can include explicit units.

Example filter entry

```json
{
  "operation": "delay",
  "properties": {
    "name": "delay",
    "value": "76 ms"
  }
}
```

#### `random_delay` [null]

_Alternative names:_ `randomdelay`, `randomDelay`.

Delays messages by a random duration. Set `distribution` or `dist` to select the distribution, and `param1` and `param2` to set its parameters. The aliases `mean`, `min`, and `alpha` set `param1`. The aliases `stddev`, `max`, and `beta` set `param2`.

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
{
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
}
```

#### `random_drop` | `randomdrop` | `randomDrop`

Drops each message with the probability specified by `prob` or `dropprob`. Set a number from zero to one.

```json
{
  "operation": "random_drop",
  "properties": {
    "name": "prob",
    "value": 0.5
  }
}
```

#### `clone` | `cloning`

Use `cloning: true` to register a cloning filter. For a filter that only copies messages, use `operation: "clone"` in both JSON and TOML. Set `delivery` to a string or an array of endpoint names to receive copies. Set `source_targets` to copy outgoing messages or `destination_targets` to copy incoming messages.

```json
{
  "name": "copy_events",
  "cloning": true,
  "operation": "clone",
  "source_targets": ["plant/events"],
  "delivery": ["temperature_sink"]
}
```

#### `firewall`

Registers a firewall operation. The configured operation passes messages by default. The filter property setters currently expose no rule configuration.

---

## Translator Options

Translators are used to bridge the gap between the value and message interfaces allowing publications to be sent to endpoints and endpoint messages to be sent to inputs as values. A translator functions as publication, input, and endpoint that other interfaces including filters can connect to. Further details can be found on the [documentation page covering translators](../user-guide/advanced_topics/translators).

_API:_ `helicsCoreRegisterTranslator`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html#a34a960d2259756912ad1dd6e70609416)
| [C](api-reference/C_API.md#translator)
| [Python](https://python.helics.org/api/capi-py/#helicsCoreRegisterTranslator)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsCoreRegisterTranslator-Tuple{HELICS.Core,%20Int64,%20String})

or

_API:_ `helicsFederateRegisterGlobalTranslator`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html#a16f2ce4896c3f5e72d353b10584102a5)
| [C](api-reference/C_API.md#translator)
| [Python](https://python.helics.org/api/capi-py/#helicsFederateRegisterGlobalTranslator)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsFederateRegisterGlobalTranslator-Tuple{HELICS.Federate,%20Int64,%20String})

---

### `name` [null]

_API:_ (none, done as part of registering the translator)

Name of the filter; must be unique to a federate.

---

### `type` [`custom`]

_API:_ (none, done as part of registering the translator)

The translator type sets the data format on its endpoint side. Accepted values are `json`, `binary`, and `custom`. The corresponding API constants are `HELICS_TRANSLATOR_TYPE_JSON`, `HELICS_TRANSLATOR_TYPE_BINARY`, and `HELICS_TRANSLATOR_TYPE_CUSTOM`. The application supplies the operator for a custom translator.

---

### `source_targets` [null]

_Alternative names:_ `sourcetargets`, `sourceTargets`

_API:_ `helicsTranslatorAddPublicationTarget`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html)
| [C](api-reference/C_API.md#translator)
| [Python](https://python.helics.org/api/capi-py/#helicsTranslatorAddPublicationTarget)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsTranslatorAddPublicationTarget-Tuple{HELICS.Translator,%20String})

or

_API:_ `helicsTranslatorAddSourceEndpoint`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html)
| [C](api-reference/C_API.md#translator)
| [Python](https://python.helics.org/api/capi-py/#helicsTranslatorAddSourceEndpoint)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsTranslatorAddSourceEndpoint-Tuple{HELICS.Translator,%20String})

Use `source_publications` for publications and `source_endpoints` for sending endpoints. The generic `source_targets` accepts either.

---

### `destination_targets` [null]

_Alternative names:_ `destinationtargets`, `destinationTargets`

_API:_ `helicsTranslatorAddInputTarget`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html)
| [C](api-reference/C_API.md#translator)
| [Python](https://python.helics.org/api/capi-py/#helicsTranslatorAddInputTarget)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsTranslatorAddInputTarget-Tuple{HELICS.Translator,%20String})

or

_API:_ `helicsTranslatorAddDestinationEndpoint`
[C++](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Translator.html)
| [C](api-reference/C_API.md#translator)
| [Python](https://python.helics.org/api/capi-py/#helicsTranslatorAddDestinationEndpoint)
| [Julia](https://julia.helics.org/latest/api/#HELICS.helicsTranslatorAddDestinationEndpoint-Tuple{HELICS.Translator,%20String})

Use `destination_inputs` for inputs and `destination_endpoints` for receiving endpoints. The generic `destination_targets` accepts either.

---

### Translator visibility, metadata, and filters

Set `global` to override `defaultglobal`. Use `endpointtype` for the endpoint-side type and `unit` or `units` for value-side units. Use `source_filters` for filters on messages sent by the translator, and `destination_filters` for filters on messages received by it. See [Interface metadata and options](#interface-metadata-and-options) for `info`, `tags`, and `flags`.

The translator loader reads `properties` in the same name/value format as filters. The built-in translators currently have no configurable operation properties.

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

## Core and Broker Options

Pass these options to the core or broker constructor, or to the broker executable. The shared C APIs are `helicsCreateCore` and `helicsCreateBroker`. Use `--config=<file>` on the command line to load options from a file. A JSON or TOML file path can also serve directly as the core or broker initialization argument. In this form, a broker reads JSON settings at the top level and in the `broker`, `helics`, `helics.helics`, and `helics.broker` sections. For options handled by the CLI configuration formatter, `--config_section` selects a named section and `--config_index` selects a zero-based entry in a configuration section represented as an array.

### Initialization, timing, and logging

- `--federates=` - The minimum total number of federates expected to join through this broker and any sub-brokers below it. Equivalent to `-f`.
- `--required_federates=` - A federate name or comma-separated list of federate names that must be registered before the broker or core enters initialization mode.
- `--max_federates=` - The maximum number of federates allowed to join through a broker.
- `--name=` - Broker or core name. Aliases are `-n`, `--identifier`, and `--uuid`.
- `--max_iterations=` - Maximum iterations allowed when using the re-iteration API
- `--min_broker_count=` - The minimum number of brokers that the co-simulation must have to begin initialization. (This option is not available for cores)
- `--max_brokers=` - The maximum number of brokers that the co-simulation should allow. (This option is not available for cores)
- `--slow_responding` - Removes the requirement for the broker to respond to pings from other entities in the co-simulation in a timely manner and forces the assumption that this broker is still connected to the federation.
- `--restrictive_time_policy` - Forces the broker to use the most restrictive (conservative) timing policy when granting times to federates. Has the potential to increase co-simulation time as time grants may happen later then they actually need to.
- `--terminate_on_error` - All errors from any member of the federation will cause the broker to terminate the co-simulation for the entire federation.
- `--force_logging_flush` - Force writing to the log after every message.
- `--log_file=` - Name of file use for logging for this broker.
- `--log_level=` - Specifies the level of logging (both file and console) for this broker.
- `--file_log_level=` - Sets the file logging level.
- `--console_log_level=` - Sets the console logging level.
- `--dumplog` - Captures a record of all logging messages and writes them out to file or console when the broker terminates.
- `--globaltime` - Specify that the broker should use a globalTime coordinator to coordinate a master clock time with all federates.
- `--asynctime` - Specify that the federation should use the asynchronous time coordinator (only minimal time management is handled in HELICS and federates are allowed to operate independently).
- `--timing = ("async"|"global"|"default"|"distributed")` - specify the timing mode to use for time coordination.
  - `distributed` - Time management is distributed and managed by each federates. This is the default
  - `global` - HELICS centrally manages the time coordination; for larger federations this is likely to be slower
  - `async` - federates manage their own time with minimal coordination from HELICS (such as when using real-time simulators)
- `--global_disconnect` - Specify that federates should wait to disconnect until the federation is completed
- `--children=` - The minimum number of child objects the broker should expect before allowing entry to the initializing state.
- `--subbrokers=` - The minimum number of child objects the broker should expect before allowing entry to the initializing state. Same as `--children` but might be clearer in some cases with multilevel hierarchies.
- `--local_federates=` - The minimum number of federates attached through direct child cores of this broker before allowing entry to the initializing state.
- `--local_subbrokers=` - The minimum number of direct child brokers, excluding cores, before allowing entry to the initializing state.
- `--brokerkey=` - A broker key to use for connections to ensure federates are connecting with a specific broker and only appropriate federates connect with the broker. See [simultaneous co-simulations](../user-guide/advanced_topics/simultaneous_cosimulations.md) for more information.
- `--profiler=log` - Send the profiling messages to the default logging file. `log` can be replaced with a path to an alternative file where only the profiling messages will be sent. See the [User Guide page on profiling](../user-guide/advanced_topics/profiling.md) for further details. If a file is specified it is cleared.
- `--profiler_append=somefile.txt` - Send the profiling messages to file and leave the existing contents appending new data. See the [User Guide page on profiling](../user-guide/advanced_topics/profiling.md) for further details.
- `--time_monitor=` - Specify the name of the federate to monitor the time from and generate periodic log messages in the broker as the federate updates its time.
- `--time_monitor_period=` - can only be used with `--time_monitor`, set the minimum time period which must elapse in simulation before another log message from the time monitor is generated
- `--logbuffer` - Enable buffering recent log messages for retrieval with the "logs" query. Optionally specify the size of the circular log buffer; defaults to 10 messages if no size is supplied.
- `--allow_remote_control` - Enables the broker to respond to certain remote commands such as "disconnect"
- `--dynamic` - Allow for dynamic federations where federates can join after the co-simulation has begun. This capability is disabled by default.
- `--disable_dynamic_sources` - Require data sources to register before initialization. Dynamic registration of data sources is allowed by default.
- `--root` - Make this broker the root of a federation, even if a parent broker is specified. Broker only.
- `--observer` - Add the core or broker as an observer.
- `--json` - Use JSON serialization for communications.
- `--disable_remote_control` - Turn off remote operations such as disconnect. Remote control is enabled by default.
- `--error_on_unmatched` - Have the broker terminate the co-simulation when it finds unmatched connections during initialization.
- `--coretype` or `-t` - Select the transport when creating a broker or core through an executable or a factory that parses its arguments. `--core` also selects the type in the core and broker factories.

### Core and broker timeouts

Use a time with units, such as `"15 s"`. Unitless values are interpreted as milliseconds. See [Timeouts](../user-guide/advanced_topics/timeouts.md) for how the timers interact.

| Option | Effect |
| --- | --- |
| `--tick` | Interval between heartbeat checks. Defaults to 5 s. |
| `--timeout` | General connection and communication timeout. |
| `--network_timeout` | Time to wait for a broker connection. |
| `--query_timeout` | Time to wait for a query response. Defaults to 15 s. |
| `--grant_timeout` | Time to wait for a grant before triggering diagnostic action. |
| `--error_delay`, `--error_timeout` | Delay before terminating after an error. |
| `--max_cosim_duration` | Wall-clock limit for an active co-simulation, checked on broker or core ticks. |
| `--disable_timer`, `--no_tick` | Disable timeout checks driven by the tick timer. |
| `--debugging` | Enable `--slow_responding` and `--disable_timer` together. |

### Core and broker connection files

Use `helicsCoreMakeConnections` or `helicsBrokerMakeConnections` to load interface connections from JSON or TOML. A file path passed directly as the core or broker initialization argument can also contain these sections. The names must match registered interfaces, including any federate prefix.

| Key | Entries |
| --- | --- |
| `connections` | Arrays of `[publication, input]` pairs, or objects/tables naming a `publication` and `targets`, an `input` and `sources`, or an `endpoint` and its `sources` and/or `targets`. |
| `links` | Arrays of `[source_endpoint, destination_endpoint]` pairs. The same named-entry forms as in `connections` are also accepted. |
| `filters` | Arrays of `[filter, source_endpoint]` pairs, or objects/tables naming a `filter` with `source_endpoints` (or `endpoints`) and/or `dest_endpoints`. |
| `globals`, `aliases` | Name-to-value mappings or lists of pairs, as in [Federate configuration sections](#federate-configuration-sections). |
| `tags` | Core metadata in JSON. See [Interface metadata and options](#interface-metadata-and-options) for supported JSON forms. |

The named connection entries accept a string or an array under the plural key, or a single string under its singular form. The `destEndpoints` and `sourceEndpoints` spellings are also accepted in filter entries.

### Environment variables

These parsers also read the following environment variables.

| Scope | Variables |
| --- | --- |
| Federate | `HELICS_CORE_TYPE`, `HELICS_CORE_INIT_STRING`, `HELICS_LOG_LEVEL` |
| Broker/core type selection | `HELICS_CORE_TYPE` |
| Broker and core | `HELICS_BROKER_KEY`, `HELICS_BROKER_LOG_LEVEL` |
| Network broker and core | `HELICS_BROKER_ADDRESS`, `HELICS_BROKER_PORT`, `HELICS_LOCAL_PORT`, `HELICS_BROKER_INIT`, `HELICS_CONNECTION_ADDRESS`, `HELICS_CONNECTION_PORT` |
| Federate and network broker/core | `HELICS_ENCRYPTION`, `HELICS_ENCRYPTION_CONFIG` |

## Network

Network options configure the transport used by a core or broker. Set them in that object's initialization options. A federate file also accepts `broker`, `broker_address`, `broker_port`, `local_port`, `port`, `autobroker`, `broker_init_string`, `encrypted`, and `encryption_config` directly.

For most HELICS users, most of the time, the following network options are not needed. They are most likely to be needed when working in complex networking environments, particularly when running co-simulations across multiple sites with differing network configurations. Many of these options require non-trivial knowledge of network operations, and it is assumed that those who need these options will understand what they do, even with the minimal descriptions given.

### `network_connectivity` [`local`]

Selects which interfaces a network core or broker uses. Values are `local`, `ipv4`, `ipv6`, `all`, and `external` (equivalent to `all`). The equivalent switches are `--local`, `--ipv4`, `--ipv6`, `--all`, and `--external`. Use either the setting or a switch.

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

### `broker_address` [transport default]

_Alternative names:_ `brokeraddress`, `brokerAddress`

_API:_ (none)

Specifies the IP address (and optionally port) a federate or sub-broker should use when contacting its parent broker

---

### `broker_port` []

_Alternative names:_ `brokerport`, `brokerPort`

_API:_ (none)

Specifies the port a federate or sub-broker should use when contacting its parent broker

---

### `broker_name` [unset]

_Alternative names:_ `brokername`, `brokerName`

Names the parent broker. The `broker` option accepts either a name or an address and looks up an in-process broker by name first. Use `broker_address` to specify an address explicitly.

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

### `port` | `-p`, `connection_address`, and `connection_port`

For network cores and brokers, `port` selects the local port and accepts `auto` for automatic allocation. For a federate, `port` sets the parent broker's port when `broker_port` is unset, or sets the local port when `broker_port` has already been set. Prefer the explicit `broker_port` and `local_port` names.

For transports that use them, `connection_address` and `connection_port` supply fallbacks when the relevant local or broker address and port are unset. These options also read `HELICS_CONNECTION_ADDRESS` and `HELICS_CONNECTION_PORT`.

### Transport-specific options

| Transport | Option | Effect |
| --- | --- | --- |
| `mpi` | `--broker_rank` or `--rank` | MPI rank of the parent broker. |
| `mpi` | `--broker_tag` or `--tag` | MPI tag of the parent broker. |
| `tcp_ss` | `--connections` | List of other TCP addresses for this core or broker to connect to. |
| `tcp_ss` | `--no_outgoing_connection` | Disable outgoing TCP connections. |

Network parsers also accept `broker_init` as an alias for `broker_init_string` and read `HELICS_BROKER_INIT` through that alias. The network `local_port` accepts `auto` for automatic allocation and `os` to request OS allocation where the transport supports it.

### `force` [false]

_Configuration:_ ZeroMQ broker initialization.

_API:_ (none)
Flag specifying that the broker network connection should attempt to override and terminate any existing broker using the specified port.
