# Configuration Options Reference

```eval_rst
.. toctree::
    :hidden:
    :maxdepth: 2
```

Many of the HELICS entities have significant configuration options. Rather than comprehensively list these options while explaining the features themselves, we've created this section of the User Guide to serve as a reference as to what they are, what they do, and how to use them.

## Configuration methods

xxxxxxx Describe the various way these key words can be used to configure the co-simulation.

### JSON configuration file

xxxxxxx content TBD

### Command-line arguments

xxxxxxx content TBD

### API parameters

xxxxxxx content TBD

## Sample Configurations

As a complement to the extensive listing of HELICS options and flags that can be used to configure HELICS to run in particular ways, the following example JSON configuration files are also provided to show specifically how this configuration can be implemented.

### Control_test.json

[This configuration](../examples/example_files/Control_test.json) is pulled from one of the User Guide examples as it utilizes [endpoints](./user-guide/message_federates.md) (for communicating control signal messages), [filters](./user-guide/filters.md) (which can modify messages _en route_) and [subscriptions](./user-guide/value_federates.md) (for receiving value signals, in this case the load of the electric vehicle on the distribution system).

```json
{
  "name": "EV_Controller",
  "loglevel": 5,
  "coreType": "zmq",
  "timeDelta": 1.0,

  "endpoints": [
    {
      "name": "EV_Controller/EV6",
      "type": "genmessage",
      "global": true
    }
  ],
  "filters": [
    {
      "name": "filterEV6",
      "target": "EV_Controller/EV6",
      "mode": "source",
      "operation": "delay",
      "properties": {
        "name": "delay",
        "value": 600
      }
    }
  ],
  "subscriptions": [
    {
      "key": "IEEE_123_feeder_0/totalLoad",
      "type": "complex",
      "required": true
    }
  ]
}
```

### example_combo_fed.json

[This configuration](../examples/example_files/example_combo_fed.json) shows the extensive use of configuration flags and demonstrates their location in the JSON file. It also shows how to configure endpoints so they receive a message every time a value signal is updated in the federation and how the info field is constructed (necessary for some simulators to use HELICS data properly).

```json
{
  "name": "comboFed",
  "observer": false,
  "rollback": false,
  "only_update_on_change": false,
  "only_transmit_on_change": false,
  "source_only": false,
  "uninterruptible": false,
  "coretype": "inproc",
  "corename": "the name of the core",
  "coreinitstring": "--autobroker",
  "max_iterations": 10,
  "period": 1.0,
  "offset": 0.0,
  "time_delta": 0.0,
  "output_delay": 0,
  "input_delay": 0,

  "endpoints": [
    {
      "name": "ept1",
      "type": "genmessage",
      "global": true
    },
    {
      "name": "ept2",
      "type": "message2",
      "knownDestinations": "ept1",
      "subscriptions": "pub1"
    }
  ],
  "publications": [
    {
      "key": "pub1",
      "type": "double",
      "unit": "m",
      "global": true
    },
    {
      "key": "pub2",
      "type": "double",
      "info": { "field1": 45, "field2": 99 }
    }
  ],
  "subscriptions": [
    {
      "key": "pub1",
      "type": "double",
      "required": true
    },
    {
      "key": "comboFed/pub2",
      "alias": "pubshortcut"
    }
  ]
}
```

## Federate Options

There are a number of flags which control how a federate acts with respect to timing and its signal interfaces.

### `name`

Every federate must have a unique name across the entire federation; this is functionally the address of the federate and is used to determine where HELICS messages are sent. An error will be generated if the federate name is not unique.

### `coreType`

There are a number of technologies or message buses that can be used to send HELICS messages among federates. Every HELICS enabled simulator has code in it that creates a core which connects to a HELICS broker using one of these messaging technologies. ZeroMQ (zmq) is the default core type and most commonly used but there are also cores that use TCP and UDP networking protocols directly (forgoing ZMQ's guarantee of delivery and reconnection functions), IPC (uses Boost's interprocess communication for fast in-memory message-passing but only works if all federates are running on the same physical computer), and MPI (for use on HPC clusters where MPI is installed). (xxxxxxxx add link to core type pages)

### `corename`

Only applicable for `ipc` and `test` core types; otherwise can be left undefined.

### `coreinit`

??? xxxxxxx

### `logfile`

??? xxxxxxx

### `log_level`

Determines the level of detail for log messages. All messages at the user-provided level and lower will be printed to the log file. Valid values are:

- -1 - no logging
- 0 - error
- 1 - warning
- 2 - summary
- 3 - connections
- 4 - interfaces
- 5 - timing
- 6 - data
- 7 - trace

### `publications` and/or `subscriptions`

These are lists of the values being sent to and from the given federate and have their own options.

### `ignore_time_mismatch_warnings` [false]

If certain timing options (_i.e._ `period`, or `minTimeDelta`) are used it is possible for the time granted a federate to be greater than the requested time. This situation would normally generate a warning message, but if this flag is set those warnings are silenced.

### `connections_required`[false] and `connections_optional` [false]

When a federate is initialized, one of its tasks is to make sure the recipients of directed signals exist. If, after the federation is initialized, the recipient can't be found, then by default a warning is generated and written to the log file. If the `connections\_required` flag is set, this warning becomes a fatal error that stops the co-simulation.

### `connections_optional` [false]

When an interface requests a target it tries to find a match in the federation. If it cannot find a match at the time the federation is initialized, then the default is to generate a warning. This will not halt the federation but will display a log message. If the `connections_optional` flag is set on a federate all subsequent `addTarget` calls on any interface will not generate any message if the target is not available.

### `strict_input_type_checking` [false]

Only applicable to Named Input interfaces ([see section on value federate interface types](./value_federates.md)), if enabled this flag checks that data type of the incoming signals match that specified for the input. (xxxxxxx - What happens if they don't match?)

### `slow_responding` [false]

If specified on a federate, setting this flag indicates the federate may be slow in responding, and to not forcibly eject the federate from the federation for the slow response. This is an uncommon scenario.

If applied to a core or broker (xxxxxxx need examples of this syntax), it is indicative that the broker doesn't respond to internal pings quickly and should not be disconnected from the federation for the slow response.

### `terminate_on_error` [false]

If the `terminate_on_error` flag is set then a federate encountering an internal error will trigger a global error and cause the entire federation to terminate. Errors of this nature are typically the result of configuration errors, such as having a required publication that is not used or incompatible units or types on publications and subscriptions.

## Interface Options

### `key`

- `publications` - The string in this field is the unique identifier (at the federate level) for the value that will be published to the federation. If `global` is set (see below) it must be unique to the entire federation.
- `subscriptions` - This string identifies the federation-unique value that this federate wishes to receive. Unless `global` has been set to `true` in the publishings JSON configuration file, the name of the value is formatted as `<federate name>/<publication key>`. Both of these strings can be found in the publishing federate's JSON configuration file as the `name` and `key` strings, respectively. If `global` is `true` the string is just the `key` value.

### `global` [false]

(publications only) `global` is used to indicate that the value in `key` will be used as a global name when other federates are subscribing to the message. This requires that the user ensure that the name is used only once across all federates. Setting `global` to `true` is handy for federations with a small number of federates and a small number of message exchanges as it allows the `key` string to be short and simple. For larger federations, it is likely to be easier to set the flag to `false` and accept the extra naming

### `required` [false]

- `publications` - At least one federate must subscribe to the publications.
- `subscriptions` - The message being subscribed to must be provided by some other publisher in the federation.

### `type`

HELICS supports data types and data type conversion ([as best it can](https://www.youtube.com/watch?v=mZOAn-3aATY)).

### `shortcut`

??? xxxxxxx

### `units`

HELICS is able to do some levels of unit conversion, currently only on double type publications but more may be added in the future. The units can be any sort of unit string, a wide assortment is supported and can be compound units such as m/s^2 and the conversion will convert as long as things are convertible. The unit match is also checked for other types and an error if mismatching units are detected. A warning is also generated if the units are not understood and not matching. The unit checking and conversion is only active if both the publication and subscription specify units.

### `info`

The `info` field is entirely ignored by HELICS and is used as a mechanism to pass configuration information to the federate so that it can properly integrate into the federation. Thus, there is no standard content or format for this field; it is entirely up to the individual simulators to decide how the data in this field (if any) should be used. Often it is used by simulators to map the HELICS names into internal variable names as shown in the above example.

## Timing Options

Below is a list of the timing parameters and flags provided by HELICS with the default values shown in brackets.

### `uninterruptible` [false]

Normally, a federate will be granted a time earlier than it requested when it receives a message from another federate; the presence of any message implies there could be an action the federate needs to take and may generate new messages of its own. There are times, though, when it is important that the federate only be granted a time (and begin simulating/executing again) that it has previously requested. For example, there could be some controller that should only operate at fixed intervals even if new data arrives earlier. In these cases, setting the `uninterruptible` flag to `true` will prevent premature time grants.

### `period` [n/a]

Many time-based simulators have a minimum time-resolution or a user-configurable step size. The `period` parameter can be used to effectively synchronize the times that are granted with the defined simulation period. The default units for `period` are in seconds but the string for this parameter can include its own units (e.g. "2 ms" or "1 hour"). Setting `period` will force all time grants to occur at times of `n*period` even if subscriptions are updated, messages arrive, or the federate requests a time between periods. This value effectively makes the federates `uninterruptible` during the times between periods. Relatedly...

### `offset` [0]

There may be cases where it is preferable to have a simulator receive time grants that are offset slightly in time to one or more other federates. Defining an `offset` value allows this to take place; units are handled the same as in `period`. Setting both `period` and `offset`, will result in the all times granted to the federate in question being constrained to `n*period + offset`.

### `timeDelta` [0]

timeDelta has some similarities to `period`; where `period` constrained the granted time to regular intervals, `timeDelta` constrains the grant time to a minimum amount from the last granted time. Units are handled the same as in `period`.

### `minTimeDelta` [0]

The minimum time delta of federate determines how close two granted times may be to each other. The default value is set to the system epsilon which is the minimum time resolution of the Time class used in HELICS. This can be used to achieve similar effects as the period, but it has a different meaning. If the period is set to be smaller than the minTimeDelta, then when granted the federate will skip ahead a couple time steps.

### `inputDelay` [0] and `outputDelay` [0]

`inputDelay` specifies a delay in simulated time between when a signal arrives at a federate and when that federate is notified that a new value is available. `outputDelay` is similar but applies to signals being sent by a federate. Note that this applies to both value signals and message signals. (xxxxxxx - Need to clarify with developers if messages are delayed t_filter + t_outputDelay or max(t_filter, t_outputDelay)

### `real_time` [false]

If set to true the federate uses `rt_lag` and `rt_lead` to match the time grants of a federate to the computer wall clock.
If the federate is running faster than real time this will insert additional delays. If the federate is running slower than real time this will cause a force grant, which can lead to non-deterministic behavior. `rt_lag` can be set to maxVal to disable force grant

### `rt_lag` [0.2] and `rt_lead` [0.2]

Defines "real-time" for HELICS by setting tolerances for HELICS to use when running a real-time co-simulation. HELICS is forced to keep simulated time within this window of wall-clock time. Most general purpose OSes do not provide guarantees of execution timing and thus very small values of `rt_lag` and `rt_lead` (less than 0.005) are not likely to be achievable.

### `source_only` [false] and `observer` [false]

To improve co-simulation performance, any federates that only send signals (`source_only`) or receive signals (`observer`) can be declared as such by setting these flags.

### `only_update_on_change` [false] and `only_transmit_on_change` [false]

Setting these flags prevents new value signals with the same value from being received by the federate or sent by the federate. Setting these flags will reduce the amount of traffic on the HELICS bus and can provide performance improvements in co-simulations with large numbers of messages.

### `wait_for_current_time_update` [false]

If set to true a federate will not be granted the requested time until all other federates have completed at least 1 iteration of the current time or have moved past it. If it is known that 1 federate depends on others in a non-cyclic fashion, this can be used to optimize the order of execution without iterating.

### `restrictive_time_policy` [false]

Using the option `restrictive-time-policy` forces HELICS to use a fully conservative mode in granting time. This can be useful in situations beyond the current reach of the distributed time algorithms. It is generally used in cases where it is known that some federate is executing and will trigger someone else, but most federates won't know who that might be. This prevents extra messages from being sent and a potential for time skips. It is not needed if some federates are periodic and execute every time step. The flag can be used for federates, brokers, and cores to force very conservative timing with the potential loss of performance as well.

## Filter Options

Filters are registered with the core or through the application API.
There are also Filter object that hide some of the API calls in a slightly nicer interface. Generally a filter will define a target endpoint as either a source filter or destination filter. Source filters can be chained, as in there can be more than one of them. At present there can only be a single non-cloning destination filter attached to an endpoint.

Non-cloning filters can modify the message in some way, cloning filters just copy the message and may send it to multiple destinations.

On creation, filters have a target endpoint and an optional name.
Custom filters may have input and output types associated with them.
This is used for chaining and automatic ordering of filters.
Filters do not have to be defined on the same core as the endpoint, and in fact can be anywhere in the federation, any messages will be automatically routed appropriately.

### `reroute`

This filter reroutes a message to a new destination. it also has an optional filtering mechanism that will only reroute if some patterns are matching the patterns should be specified by "condition" in the set string the conditions are regular expression pattern matching strings

### `delay`

This filter will delay a message by a certain amount

### `randomdelay`

This filter will randomly delay a message according to specified random distribution
available options include distribution selection, and 2 parameters for the distribution
some distributions only take one parameter in which case the second is ignored. The distributions available are based on those available in the C++ [random](http://www.cplusplus.com/reference/random/) library

- **constant** - param1="value" this just generates a constant value
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

### `randomdrop`

This filter will randomly drop a message, the drop probability is specified, and is modeled as a uniform distribution.

### `clone`

this message will copy a message and send it to the original destination plus a new one.

### `firewall`

The firewall filter will eventually be able to execute firewall like rules on messages and perform certain actions on them, that can set flags, or drop or reroute the message. The nature of this is still in development and will be available at a later release.

## custom filters

(xxxxxxx need to add more details)
Custom filters are allowed as well, these require a callback operator that can be called from any thread
and modify the message in some way.
