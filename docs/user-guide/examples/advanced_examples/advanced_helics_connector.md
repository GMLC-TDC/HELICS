# helics_connector

This example demonstrates the use of the "helics_connector" app to create interface connection between federates outside of the configuration JSON or API calls.

- [helics_connector](#helics_connector)
  - [Where is the code?](#where-is-the-code)
  - [What is this co-simulation doing?](#what-is-this-co-simulation-doing)
    - [Differences compared to the Advanced Default example](#differences-compared-to-the-advanced-default-example)
      - [HELICS Differences](#helics-differences)
    - [HELICS components](#helics-components)
      - [Configuration by match-file](#configuration-by-match-file)
      - [Match-file specification](#match-file-specification)
      - [Configuration by interface creation](#condfiguration-by-interface-creation)
        - [Query callback](#query-callback)
        - [Configuration command response](#configuration-command-response)
        - [Interface creation timing](#interface-creation-timing)
  - [Execution and Results](#execution-and-results)
  - [Questions and Help](#questions-and-help)

## Where is the code?

This example code on [the use of helics_connector can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_connector). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_connector_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_connector)

## What is this co-simulation doing?

This example shows how to use helics_connector to create connections between HELICS federate interfaces without specifying said connections via the JSON configuration or similar APIs. Specifically, only publications (with no targets defined), outputs (with no source defined), and endpoints need to be defined in the JSON. helics_connector then uses a match-file to establish the connections between publication outputs and inputs.

### Differences compared to the Advanced Default example

This example has the same federates interacting in the same ways as in the [Advanced Default example](./advanced_default.md). The only difference is the use of helics_connector to create the data exchanges between the federates.

#### HELICS Differences

By configuring the federates without any targeting in the input or output, helics_connector uses existing public APIs to establish the data exchanges between federates. No core library changes were made to add this functionality and it is entirely possible for other HELICS users to write their own versions of helics_connector to satisfy their particular needs.

### HELICS components

#### Configuration by match-file

The only difference between the federation as defined in the "advanced default" example and this example is how the configuration is defined. The advanced_default example uses subscriptions instead of inputs in the configuration JSON while this example uses inputs. For example, a portion of the Battery federate configuration for the advanced_default example and this example are shown below; the advanced_default configuration uses subscriptions and specifies a "key" as a data source where this example uses inputs and has no "key" value.

```json
"subscriptions":[
    {
      "key":"Charger/EV1_voltage",
      "type":"double",
      "unit":"V",
      "global": true
    }
]
```

```json
"inputs": [
    {
      "global": true,
      "name": "Battery/EV100_input_voltage",
      "type": "double",
      "unit": "V"
    }
]
```

#### Match-file specification

The match-file can be specified either using a plain textfile or JSON. The simplest format specifies the individual connections between federates, one connection per line:

```text
Charger/EV1_output_voltage Battery/EV1_input_voltage from_to
Charger/EV2_output_voltage Battery/EV2_input_voltage from_to
Charger/EV3_output_voltage Battery/EV3_input_voltage from_to
Charger/EV4_output_voltage Battery/EV4_input_voltage from_to
Charger/EV5_output_voltage Battery/EV5_input_voltage from_to
Charger/EV1_input_current Battery/EV1_output_current to_from
Charger/EV2_input_current Battery/EV2_output_current to_from
Charger/EV3_input_current Battery/EV3_output_current to_from
Charger/EV4_input_current Battery/EV4_output_current to_from
Charger/EV5_input_current Battery/EV5_output_current to_from
```

In JSON, the format looks like:

```json
{
  "connections": [
    ["Charger/EV1_output_voltage", "Battery/EV1_input_voltage", "from_to"],
    ["Charger/EV2_output_voltage", "Battery/EV2_input_voltage", "from_to"],
    ["Charger/EV3_output_voltage", "Battery/EV3_input_voltage", "from_to"],
    ["Charger/EV4_output_voltage", "Battery/EV4_input_voltage", "from_to"],
    ["Charger/EV5_output_voltage", "Battery/EV5_input_voltage", "from_to"],
    ["Charger/EV1_input_current", "Battery/EV1_output_current", "to_from"],
    ["Charger/EV2_input_current", "Battery/EV2_output_current", "to_from"],
    ["Charger/EV3_input_current", "Battery/EV3_output_current", "to_from"],
    ["Charger/EV4_input_current", "Battery/EV4_output_current", "to_from"],
    ["Charger/EV5_input_current", "Battery/EV5_output_current", "to_from"]
  ]
}
```

The "from-to" or "to-from" is used to indicate which side of the connection is un-named and thus is the producer of the data ("from") and which side is named and is the target for the data ("to"). helics_connector goes through and finds all **unconnected** interfaces on the federates themselves, finds the corresponding interface indicated as a "from" connection in the match-file, and then connects them with any existing entry in the match-file on the "to" side (even if they already have a target).

Alternatively, the match-file can be specified using regular expressions. This can be a convenient way to specify a large number of interface connections if they follow a naming convention. The regular expression format takes advantage of the ability to name terms in the expression, allowing terms in one part of the expression to be used later on in the expression. In the case, the above match-file looks like this:

```text
# this is a comment
REGEX:Charger/EV(?<ev_num>\d*)_output_voltage, REGEX:Battery/EV(?<ev_num>)_input_voltage, from_to
REGEX:Charger/EV(?<ev_num>\d*)_input_current, REGEX:Battery/EV(?<ev_num>)_output_current, from_to
```

In this case, "ev_num" is the name given to the numerals that appear after the characters "EV" and are used in both first and second terms of the match-file. Each regular expression must be prefixed with the "REGEX:" string.

Its possible to mix direct connections and regular expressions in the same matchfile but not because the helics_connector only works on unconnected interfaces, there is an implied precedence in the match-file that works top down. That is, connections at the top of the file over-ride those specified at the bottom of the file. Once an interface marked as a "from" is connected, it is not eligible for connections defined later in the file.

#### Configuration by interface creation

A second, more complex example is also included where helics_connector facilitates a custom communication protocol between federates where the interfaces themselves are created on demand. In this case, the federates using this protocol must be written to respond to a specific query where they indicate the interfaces they could create and then respond to a specific command from helics_connector by creating the interfaces indicated by helics_federate. In both cases helics_connector uses a match-file specification to define which interfaces to create.

The configuration JSON of the federate is quite different as no interfaces are defined and only timing information is included:

```json
{
  "name": "Battery",
  "core_type": "zmq",
  "log_level": "warning",
  "period": 60.0,
  "uninterruptible": false,
  "terminate_on_error": true,
  "wait_for_current_time_update": true
}
```

##### Query callback

The federate code needs to include the necessary functionality to define a callback to handle custom queries. The example shown here is in Python and the HELICS Python library is C-based so setting up the callback correctly is more challenging than most of the Python code in these examples.

To pass information into the callback (which is never explicitly called and thus it is not possible to pass specific parameters), a "UserData" class is defined that allows the inclusion of arbitrary data (in this case, the number of EVs). This has been defined as a value that is passed in when the object is instantiated ("num_EVs"). This value is used to define a class attribute, thus preserving it in the object itself.

```python
class UserData(object):
    def __init__(self, num_EVs):
        self.num_EVs = num_EVs
        userdata = h.ffi.new_handle(self)
        self._userdata = userdata
```

The function that is called when the query is made. In our example, we called it "query_callback" and it has a decorator of the callback function in C that must match the signture of said function.

```python
@h.ffi.callback(
    "void query(const char *query, int querySize, HelicsQueryBuffer buffer, void *user_data)"
)
def query_callback(query_ptr, size: int, query_buffer_ptr, user_data):
    ...
```

The query being sent to this federate is defined in the "query_buffer_ptr" variable but must be decoded into a string by declaring it a HELICS buffer object and then decoding it into a string:

```python
query_buffer = h.HelicsQueryBuffer(query_buffer_ptr)
query_str = h.ffi.string(query_ptr, size).decode()
```

After doing all this work, we can see that the query sent is "potential_interfaces"; this is the query sent by helics_connector to find out what potential interfaces each federate can create. To answer this query we need to pull in the number of EVs passed in through the UserData object

```python
num_EVS = h.ffi.from_handle(user_data).num_EVs
```

The query response is a JSON dictionary composed of three lists: the federates publications, inputs, and endpoints. In this case, the name of the interfaces can be created easily but must match the names used in the helics_connector match-file. The dictionary is used to fill the HELICS buffer object that was sent in, effectively responding to the query.

```python
for EVnum in range(1, num_EVs + 1):
    pubs.append(f"Battery/EV{EVnum}_output_current")
    inputs.append(f"Battery/EV{EVnum}_input_voltage")
response_dict = {"publications": pubs, "inputs": inputs, "endpoints": []}
query_response = json.dumps(response_dict)
h.helicsQueryBufferFill(query_buffer, query_response)
```

With "UserData" and "query_callback" defined, they now just need to be included in the main body of the federate code. An instance of the UserData object is created ("user_data") using the number of EVs and a handle (pointer) to the object is defined. Lastly, the query callback is defined via a HELICS API, referencing both "query_callback" and "user_data".

```python
user_data = UserData(num_EVs)
user_data_handle = h.ffi.new_handle(user_data)
h.helicsFederateSetQueryCallback(fed, query_callback, user_data_handle)
```

##### Configuration command response

After responding to the query, the federate also needs to respond to the command sent by helics_connection to define the interfaces it needs to create. helics_connector sends a JSON as a command with the "command" field of that object being "register_interfaces". The rest of the object has three fields: the publications, inputs, and endpoints the object needs to create.

```json
{
  "command": "register_interfaces",
  "inputs": [
    "Battery/EV5_input_voltage",
    "Battery/EV4_input_voltage",
    "Battery/EV3_input_voltage",
    "Battery/EV2_input_voltage",
    "Battery/EV1_input_voltage"
  ],
  "publications": [
    "Battery/EV5_output_current",
    "Battery/EV4_output_current",
    "Battery/EV3_output_current",
    "Battery/EV2_output_current",
    "Battery/EV1_output_current"
  ]
}
```

The example code checks to make sure the interfaces are provided as lists and then creates the interfaces as a part of the custom "register_interfaces_from_command" function. In this case, we assume all inputs are doubles; a more general command could include data types.

```python
if isinstance(cmd["publications"], list):
    for pub in cmd["publications"]:
        h.helicsFederateRegisterGlobalPublication(fed, pub, h.HELICS_DATA_TYPE_DOUBLE)
if isinstance(cmd["inputs"], list):
    for inp in cmd["inputs"]:
        h.helicsFederateRegisterGlobalInput(fed, inp, h.HELICS_DATA_TYPE_DOUBLE)
```

##### Interface creation timing

The above defined query response and command protocol all takes place during the initialization phase of the federate operation. To accomplish multiple asynchronous events in initializing mode, the ["iterative" version of entering initialization mode](https://python.helics.org/api/capi-py/#helicsFederateEnterInitializingMode) needs to be used. Due to the way HELICS grants time in initializing mode, the query is guaranteed to be available after enter initializing mode. This is handled via the callback and not explicit call needs to be made. To guarantee the command message is available for acting on, the same iterative initializing mode call is made again. After this, the command is deciphered, the interfaces created, and, finally, the federate enters executing mode.

```python
h.helicsFederateEnterInitializingModeIterative(fed)
# Query is guaranteed to be available after this call but may be
# available earlier. Callback responds whenever the query comes in.
h.helicsFederateEnterInitializingModeIterative(fed)
command = h.helicsFederateGetCommand(fed)
if len(command) == 0:
    raise TypeError("Empty command.")
try:
    logger.debug(f"command string: {command}")
    cmd = json.loads(command)
except:
    raise TypeError("Not able to convert command string to JSON.")
register_interfaces_from_command(fed, cmd)  # custom function
h.helicsFederateEnterExecutingMode(fed)
```

## Execution and Results

Run the co-simulation, use one of the following:

```shell
$ helics run --path=./advanced_matchfile_direct_runner.json
$ helics run --path=./advanced_matchfile_regex_runner.json
$ helics run --path=./advanced_connector_interface_creation_runner.json
```

Since this is only a change to the configuration method of the federation, the results are identical to those in the [Advanced Default example.](./advanced_default.md)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_charging_power.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_estimated_SOCs.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_battery_SOCs.png)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
