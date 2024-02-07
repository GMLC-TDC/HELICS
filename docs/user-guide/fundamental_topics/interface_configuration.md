# Federate Interface Configuration

As soon as one particular instance of a simulator begins running in a co-simulation it is a federate. Every federate will require configuration of the way it will communicate (send signals) to other federates in the federation. For [simulators that already have HELICS support](../../references/Tools_using_HELICS.md), the configuration takes the form of a JSON (or TOML) file; bespoke simulators can be configured with the HELICS APIs in the code or via a JSON file. The essential information required to configure federate interfaces with HELICS is:

**Federate name** - The unique name this federate will be known as throughout the federation. It is essential this name is unique so that HELICS messages can route properly.

**Core type** - The core manages interfaces between the federation and the federate; there are several messaging technologies supported by HELICS.

**Publications and Inputs** - Publication configuration contains a listing of source interface, data types, and units being sent by the federate; input configuration does the same for values being received by the federate. If supported by the simulator (e.g., [a Python simulator](../examples/fundamental_examples/fundamental_default.md)), these values can be mapped to internal variables of the simulator from the configuration file.

**Endpoints** - Endpoints are sending and receiving points for HELICS messages to and from message federates. They are declared and defined for each federate.

**Time step size** - This value defines the resolution of the simulator to prevent HELICS from telling the simulator to step to a time of which it has no concept (e.g. trying to simulate the time of 1.5 seconds when the simulator has a resolution of one second).

This section describes how to configure the federate interfaces using JSON files and API calls. Extensive details on the options for configuring HELICS federates is available in the [Configurations Options Reference](../../references/configuration_options_reference.md). If the user has written the simulator, it may be preferable to use the HELICS APIs to configure the federates, because the interface registrations can be made into a variable of the simulation. For non-open-source simulators, the JSON configuration files must be written before the federation is launched.

- [JSON Configuration](#json-configuration)
- [API Configuration](#api-configuration)

## JSON Configuration

Federate interfaces must be configured with JSON files if they are built from non-open-source tools, such as the simulators listed in the reference page on [Tools with HELICS Support](../../references/Tools_using_HELICS.md). Federates built from simulators written by the user can be configured with JSON files or API calls.

The [Examples](../examples/examples_index.md) illustrate in detail how to integrate federates built from open source tools, such as Python. The [Fundamental Combination Federation](../examples/fundamental_examples/fundamental_combo.md) configures the Python federate interfaces with JSON files. In this co-simulation, there are three federates: a value Battery federate, a combination Charger federate, and a message Controller federate. The example JSON shows the interface configuration for the combination federate to illustrate the different types of interfaces.

### Sample JSON configuration file

Below is a sample JSON configuration file with some of the more common options. There are many, many more configuration parameters that this file could include; a relatively comprehensive list along with explanations of the functionality they provide can be found in the [Configurations Options Reference](../../references/configuration_options_reference.md).

```json
{
  "name": "sample_federate",
  "loglevel": "debug",
  "coreType": "zmq",
  "period": 60,
  "offset": 10,
  "uninterruptible": false,
  "terminate_on_error": true,
  "wait_for_current_time_update": false,
  "federate_init_string": "--broker_address=127.0.0.1 --port=23405"
  "endpoints": [
    {
      "name": "sample_federate/ep1",
      "destination": "other_federate/ep1",
      "global": true
    }
  ],
  "publications":[
    {
      "key":"sample_federate/voltage",
      "type":"double",
      "unit":"V",
      "global": true,
      "only_transmit_on_change": true,
      "tolerance": 0.1,
      "tags": {
        "period": 0.5,
        "description": "a test publication"
      }
    }
  ],
  "subscriptions":[
    {
      "key":"other_federate/current",
      "type":"double",
      "unit":"A",
      "global": true,
      "only_update_on_change": true,
      "tolerance": 0.2,
      "default":0.91
    }
  ],
  "inputs": [
    {
      "key": "ipt2",
      "type": "double",
      "connection_required": true,
      "target": "pub1",
      "global": true,
      "default":"3.67",
      "tags": [
        { "name": "period", "value": "0.7" },
        { "name": "description", "value": "a test input" }
      ]
    }
    //specify an input with a target multiple targets could be specified like "targets":["pub1","pub2","pub3"]
  ],
}
```

### JSON configuration file explanation

- **`name`** - Every federate must have a unique name across the entire federation; this is functionally the address of the federate and is used to determine where HELICS messages are sent. An error will be generated if the federate name is not unique.
- **`loglevel`** - The level of detail exported to the [log files](./logging.md) during run time ranges from "none" to "trace".
- **`coreType`** - There are a number of technologies or message buses that can be used to send HELICS messages among federates, detailed in [Core Types](../advanced_topics/CoreTypes.md). Every HELICS enabled simulator has code in it that creates a core which connects to a HELICS broker using one of these messaging technologies. ZeroMQ (zmq) is the default core type and most commonly used, but there are also cores that use TCP and UDP networking protocols directly (forgoing ZMQ's guarantee of delivery and reconnection functions), IPC (uses Boost's interprocess communication for fast in-memory message-passing but only works if all federates are running on the same physical computer), and MPI (for use on high-performance computing clusters where MPI is installed).
- **`period`** and **`offset`** - The federate needs instruction for how to step forward in time in order to synchronize calculations. This is the simplest way to synchronize simulators to the same time step; this forces the federate to time step intervals of `n*period + offset`. The default units are in seconds. Timing configuration is explained in greater detail in the [Timing](./timing_configuration.md) page, with additional configuration options in the [Configuration Options Reference](../../references/configuration_options_reference.md#timing-options).
- **`uninterruptible`** - Setting `uninterruptible` to `false` allows the federate to be interrupted if there is a signal available for it to receive. This is a [timing configuration](./timing_configuration.md) option.
- **`terminate_on_error`** - By default, HELICS will not terminate execution of every participating federate if an error occurs in one. However, in most cases, if such an error occurs, the cosimulation is no longer valid. Setting `terminate_on_error` frees the federate from the broker if there is an error in execution, which simplifies debugging. This will prevent your federate from hanging in the event that another federate fails.
- **`wait_for_current_time_update`** - There are times when HELICS will grant the same simulated time to a number of federates simultaneously. There is a possibility of this leading to unexpected co-simulation results if federates are unexpectedly operating on old data. Using this flag, HELICS uses this option to provide the ability for one federate to always be granted this time last, after all other federates that have been granted this time have requested a later time. This ensures that the federate with this flag set will have all the latest information from all other federates before it begins execution at the granted time.
- **`federate_init_string`** - This option provides a way of passing in a large number of configuration options that a federate needs during initialization. You can consult the [Configuration Options Reference](../../references/configuration_options_reference.md#broker_init_string--null) page for a more complete list but there are a few worth bringing up specifically. `--broker_address=<IP address>` and `--port=<port number>`- Allows you to specify the IP address and port number of the broker to which you want this federate to connect. You can consult the [Advanced Topics section of the User Guide](../advanced_topics/advanced_topics_index.md) to see further explanation of how to handle more complex broker configuration.
- **`endpoints`**
  - `name` - The string in this field is the unique identifier for the endpoint interface.
  - `destination` - This option can be used to set a default destination for the messages sent from this endpoint. The default destination is allowed to be rerouted or changed during run time.
  - `global` - Just as in value federates, `global` allows for the identifier of the endpoint to be declared unique for the entire federation.
- **`publications`**
  - `key` - The string in this field is the unique identifier (at the federate level) for the value that will be published to the federation. In the example above, `global` is set to `true`, meaning the `key` must be unique to the entire federation.
  - `global` - Indicates that the value in `key` will be used as a global name when other federates are subscribing to the message. This requires that the user ensure that the name is used only once across all federates. Setting `global` to `true` is handy for federations with a small number of federates and a small number of message exchanges as it allows the `key` string to be short and simple. For larger federations, it is likely to be easier to set the flag to `false`.
  - `required` - At least one federate must subscribe to the publications.
  - `type` - Data type, such as integer, double, complex.
  - `units` - The units can be any sort of unit string, a wide assortment is supported and can be compound units such as m/s^2 and the conversion will convert as long as things are convertible. The unit match is also checked for other types and an error if mismatching units are detected. A warning is also generated if the units are not understood and not matching. The unit checking and conversion is only active if both the publication and subscription specify units. HELICS is able to do some levels of unit conversion, currently only on double type publications but more may be added in the future.
  - `only_transmit_on_change` and `tolerance` - Publications will only send a new value out to the federation when the value has changed more than the delta specified by `tolerance`.
  - `alias` - an alternate name for the publication must be globally unique for publications
  - `tags` - Arbitrary string value pairs that can be applied to interfaces. Tags are available to others through queries but are not transmitted by default. They can be used to store additional information about an interface that might be useful to applications. At some point in the future automated connection routines will make use of them. "tags" are applicable to any interface and can also be used on federates.
- **`subscriptions`** - These are lists of the values being sent to and from the given federate.

  - `key` - This string identifies the federation-unique value that this federate wishes to receive. If `global` has been set to `false` in the `publications` JSON configuration file, the name of the value is formatted as `<federate name>/<publication key>`. Both of these strings can be found in the publishing federate's JSON configuration file as the `name` and `key` strings, respectively. If `global` is `true` the string is the publishing federate's `key` value.
  - `required` - The message being subscribed to must be provided by some other publisher in the federation.
  - `type` - Data type, such as integer, double, complex.
  - `units` - Same as with `publications`.
  - `global` - Applies to the `key`, same as with `publications`.
  - `default` - set the default value to return if no publications have been received
  - `only_update_on_change` and `tolerance` - Subscriptions will only consider a new value received when that value has changed more than the delta specified by `tolerance`.

- **`inputs`** - These are lists of the values being sent to and from the given federate.
  - `name` - the name of the input.
  - `required` - The input must have a valid target
  - `type` - Data type, such as integer, double, complex.
  - `units` - Same as with `publications`.
  - `global` - Applies to the `key`, same as with `publications`.
  - `default` - set the default value to return if no publications have been received
  - `target` - A key for a publication the input should receive, may be an array such as ["pub1","pub2","pub3"]
  - `tags` - name and value pairs defining user tags for the interface
  - `only_update_on_change` and `tolerance` - Inputs will only consider a new value received when that value has changed more than the delta specified by `tolerance`.

## API Configuration

Configuring the federate interface with the API is done internal to a user-written simulator. The specific API used will depend on the language the simulator is written in. Native APIs for HELICS are available in [C++](../../references/api-reference/CPP.md) and [C](../../references/api-reference/C_API.md). MATLAB, Java, Julia, Nim, and Python all support the C API calls (ex: `helicsFederateEnterExecutionMode()`). Python and Julia also have native APIs (see: [Python (PyHELICS)](https://python.helics.org/api/), [Julia](https://gmlc-tdc.github.io/HELICS.jl/latest/api/)) that wrap the C APIs to better support the conventions of their languages. The [API References](../../references/api-reference/index.md) page contains links to the APIs.

The [Examples](../examples/examples_index.md) in this User Guide are written in Python -- the following federate interface configuration guidance will use the [PyHELICS](https://python.helics.org/api/) API, but can easily be adapted to other C-based HELICS APIs.

### Sample PyHELICS API configuration

The following example of a federate interface configuration with the PyHELICS API comes from the [Fundamental Integration Example](../examples/fundamental_examples/fundamental_fedintegration.md). This co-simulation has exactly the same interface configuration as the Combination Federation above. The only difference is that the federate interfaces are configured with the PyHELICS API.

In the `Charger.py` simulator, the following function calls the APIs to create a federate:

```python
def create_combo_federate(fedinitstring, name, period):
    fedinfo = h.helicsCreateFederateInfo()
    # "coreType": "zmq",
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
    # "loglevel": 11,
    h.helicsFederateInfoSetIntegerProperty(fedinfo, h.helics_property_int_log_level, 11)
    # "period": 60,
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_period, period)
    # "uninterruptible": false,
    h.helicsFederateInfoSetFlagOption(fedinfo, h.helics_flag_uninterruptible, False)
    # "terminate_on_error": true,
    h.helicsFederateInfoSetFlagOption(fedinfo, h.HELICS_FLAG_TERMINATE_ON_ERROR, True)
    # "name": "Charger",
    fed = h.helicsCreateCombinationFederate(name, fedinfo)
    return fed
```

The interface configurations are finalized and registered in one step using the following APIs:

```python
fedinitstring = " --federates=1"
name = "Charger"
period = 60
fed = create_combo_federate(fedinitstring, name, period)

num_EVs = 5
end_count = num_EVs
endid = {}
for i in range(0, end_count):
    end_name = f"Charger/EV{i+1}.so"
    endid[i] = h.helicsFederateRegisterGlobalEndpoint(fed, end_name, "double")
    dest_name = f"Controller/ep"
    h.helicsEndpointSetDefaultDestination(endid[i], dest_name)

pub_count = num_EVs
pubid = {}
for i in range(0, pub_count):
    pub_name = f"Charger/EV{i+1}_voltage"
    pubid[i] = h.helicsFederateRegisterGlobalTypePublication(
        fed, pub_name, "double", "V"
    )

sub_count = num_EVs
subid = {}
for i in range(0, sub_count):
    sub_name = f"Battery/EV{i+1}_current"
    subid[i] = h.helicsFederateRegisterSubscription(fed, sub_name, "A")
```

### PyHELICS API configuration explanation

All the API calls reference the PyHELICS library with

```python
import helics as h
```

#### Federate Creation **`create_combo_federate()`**

- **`h.helicsCreateFederateInfo()`** - Sets the federate information variable (set to `fedinfo`)
- **`h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")`** - Sets the core type for `fedinfo` to `zmq`
- **`h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)`** - Sets the number of federates (`fedinitstring` has been passed as `" --federates=1"`)
- **`h.helicsFederateInfoSetIntegerProperty()`** - Sets log level calling another API, `h.helics_property_int_log_level`
- **`h.helicsFederateInfoSetTimeProperty()`** - Sets time information. This API must receive another API to distinguish which type of time property to set. The period is set with `h.helics_property_time_period`, and `period` has been pass to this function
- **`h.helicsFederateInfoSetFlagOption()`** - API to set a flag for the federate. The flag we are setting is `h.helics_flag_uninterruptible` to `False`, to mirror the JSON configuration
- **`h.helicsFederateInfoSetFlagOption()`** - API to set a flag for the federate. The flag we are setting is `h.HELICS_FLAG_TERMINATE_ON_ERROR` to `True`
- **`fed = h.helicsCreateCombinationFederate(name, fedinfo)`** - Creates the combination federate with the name passed to this function (`Charger`) and the information set above for `fedinfo`

#### Federate Interface Configuration and Registration

- **Endpoints**
  - **`h.helicsFederateRegisterGlobalEndpoint(fed, end_name, 'double')`** - The `fed` has been created, `end_name` is set in a loop, and the endpoint is registered as global `double`. This API registers the id object for each endpoint, `endid[i]`
  - **`h.helicsEndpointSetDefaultDestination(endid[i], dest_name)`** - As with the JSON configuration, a default destination is set with a destination name, `'Controller/ep'`, for each endpoint object
- **Publications**
  - **`h.helicsFederateRegisterGlobalTypePublication(fed, pub_name, 'double', 'V')`** - The publication interfaces are registered for the `fed` by looping through `pub_name`. The interface is given a datatype of `double`, units of `V` for volts, and designated as global type
- **Subscriptions**
  - **`h.helicsFederateRegisterSubscription(fed, sub_name, 'A')`** - The subscription interfaces are registered for the `fed` by looping through `sub_name`. The interface is given units of `A` for amps. Alternatively, the PyHELICS API for Inputs can be used: `h.helicsFederateRegisterGlobalTypeInput(fed, sub_name, 'double','A')`

Interface configuration, including federate creation and registration, is done prior to the co-simulation execution. The next section in this User Guide places federate interface configuration in the context of the co-simulation stages and discusses the four stages of the co-simulation.
