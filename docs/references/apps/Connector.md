# Connector

The Connector app can automatically connect interfaces together. It does this using the query mechanisms inside HELICS to detect all the unconnected interfaces in a cosimulation. Then using given configuration rules it will establish connections between those interfaces.

It can also run in a two-phase mode to have the federates create then connect interfaces. In the first phase, the connector app will query the federates for their potential interfaces and then go through those interfaces to see if there is a potential connection to be made using the same rules. If a potential interface has a connection available, it will send a command to the appropriate federate to create the interface. Once the interfaces exist, the Connector enters the second phase and makes the requested connections from the new and existing unconnected interfaces.

## Connector configuration

The main mechanism to load connection information is through configuration files called "match-files", typically a plain text file. The format is:

`<origin> <target> <*direction> <*tags...>`

"origin" is the interface that is currently unconnected and "target" is the interface to connect it to. "direction" is optional and is assumed to be bidirectional matching ("bi"); in this case the direction of the match does not indicate the flow of the data but rather which interface is unmatched (the "from"). For example, if the match string looked like "V_out, V_in from_to" the Connector treats "V_out" as unconnected and will match it with "V_in" even if "V_in" is already connected. Bidrectional matching allows either interface to be unconnected to create a match.

"tags" are optional and allow for filtering the candidate connections; see below for further details on their use. Comments lines are supported and begin with `#`. Currently only publications, inputs, and endpoints are supported for matching by Connector.

Here's a simple example of a plain text match-file.

```text
#comment line for simple file test
inp1 pub1 from_to
```

The following example uses a cascaded matching definition.

```text
# comment line for cascade file test
# second comment line
inp1 intermediate1
intermediate1 intermediate1 bi
intermediate1 intermediate2 from_to
intermediate2 intermediate3 from_to
# comment line in the middle
intermediate3 pub1 bi
inp2 intermediate2 from_to
publication3 input3 to_from
```

The following example uses tags in the matching process, see more on tags below.

```text
#comment line for simple file test
inp1 pub1 from_to tag1
inp2 pub1 tag2 tag3
```

The match-file can also be JSON formatted.

```json
{
  "connections": [
    ["inp1", "pub1", "FROM_TO", "tag1"],
    ["inp2", "pub2", "tag2", "tag3"]
  ]
}
```

## Notes on Tags

- Connections specified with no tags or "default" tag will match with everything as if the tag were not there. If a connection specified by a match in the match-file uses a tag, a connection will only be made if the specified tag is used by a federate, core, or broker.
- A tag can be specified by a "global_value". The tag used for the connector is the name of the global or tag and the value can be anything other than a "false" value; if the tag is specified with a "false" value it is not used in the matching. Tags used in the match-file can also be specified in the value of the "tags" global or local tag. In this case they are specified with a comma separated list. The complete list of "false" valued strings is as follows:

```text
     "0",        "",         "false",
    "False",    "FALSE",    "off",
    "Off",      "OFF",      "disabled",
    "Disabled", "DISABLED", "disable",
    "Disable",  "DISABLE",  "f",
    "F",        "0",        std::string_view(reinterpret_cast<const char*>(&nullstringRep), 1),
    " ",        "no",       "NO",
    "No",       "-"
```

## Regular Expression (regex) Matching

In addition to directly defining the connections to be made between interfaces, one-by-one, it is also possible to use regular expressions to define the connections. Regular expressions allow a large number of similarly-name interfaces to be matched with a single statement. The file is formatted as follows:

```text
REGEX:pub_num_(?<interface_num>\d*)_(?<alpha_index>[A-Za-z]*), REGEX:input_num_(?<interface_num>\d*)_(?<alpha_index>[A-Za-z]*)
```

In this example, "interface_num" and "alpha_index" are user-defined strings that gives a name to the portion of the regex that needs to match. The funny stuff immediately after it ("\d*", "[A-Za-z]*") is the regular expression proper that the regular expression will use to determine how to make matches. Using the above example, the following matches would be made:

```text
pub_num_1_a input_num_1_a
pub_num_204_voltage input_num_204_voltage
```

Writing regular expressions quickly and accurately is a learned skill and depending the names of the interfaces, it can be difficult to craft one that does exactly what you need. The use of tags may be helpful in preventing matches between federates when they are not needed. Additionally, it may be easier to write a regular expression that makes most of the matches you need and then use direct matches for the remainder.

## Interface Creation and Matching

As mentioned in the introduction, it is also possible for the Connector app to interact with federates that are created with no exposed interfaces and jointly work through a process where those interfaces are created and then connected. A [Python example](https://github.com/GMLC-TDC/HELICS-Examples/tree/53bece298f9be952002e2f9201f24922fabc73b4/user_guide_examples/advanced/advanced_connector/interface_creation) of this process in action can be found in the [HELICS Examples repository](https://github.com/GMLC-TDC/HELICS-Examples) but a conceptual overivew of the process is as follows:

### Federate creation

On launch of the federation, the federates are created with no exposed interfaces BUT with an understanding of what interfaces it can create. These interfaces may, for example, be hard-coded or based on the system model it reads on start-up.

### Interface Query

The Connector queries the federates to determine which interfaces each one can create. The query is made after the federate enter initializing mode and the federate must enter initializing mode iteratively (`helicsFederateEnterInitializingModeIterative()`) to synchronize the query responses across the federation. Every federate that is going to create interfaces needs to register a callback function to handle this custom query by the Connector and respond appropriately. The Connector will query the federate with "potential_interfaces" and the federate must respond with a properly formatted JSON:

```json
{
  "publications": [<list of names of publications that can be created>]
  "inputs": [<list of names of inputs that can be created>]
  "endpoints": [<list of names of endpoints that can be created>]
}
```

As this is a query operation, which are executed asynchronously with the simulation time, it is undefined when the query will be made and thus a callback function must be used to respond to the query.

### Connector Interface Creation Command

After receiving the query responses from all the federates, the connector performs its standard matching operation using a match-file. Once the matches are made, it determines which connections need to be made and sends a command to each federate telling it which interfaces to create. As with the query, the commands are received asynchronously but are guaranteed to be present after calling `helicsFederateEnterInitializingModeIterative()` twice. At that point, the federate can get the command and parse the returned JSON to determine which interfaces to create. The format is the same as the query response:

```json
{
  "publications": [<list of names of publications to be created>]
  "inputs": [<list of names of inputs to be created>]
  "endpoints": [<list of names of endpoints to be created>]
}
```

### Interface Creation and Co-Simulation Execution

The federate takes the JSON command and, using its own internal knowledge of the interface (global or not, data type, units) and creates the interfaces. After that, the federate doesn't need to do anything else for the interface connections to be connected and can call `helicsFederateEnterExecutingMode()` (assuming it has nothing else to do as a part of initializing). The Connector will make the connections between the interfaces as they are created and when complete, exit the federation.

## Use of the Connector

To use the Connector to create the interface connections, simply call it as part of your federation, adding the matchfile as a command-line argument. The connector app will start up when the federation is launched and, using the match-file, create the connections between interfaces behind the scenes. Once the work it complete (by the "execution" mode of the federation), it exits the federation and allows now connected federates to proceed. A sample call looks like:

```sh
helics_connector matchfile.txt
```

## Command line arguments

Options specific to the connector are as follows:

```text
Options:
  --version                   Display program version information and exit
  --connection [INTERFACE1,INTERFACE2,DIRECTIONALITY,TXT...] ...
                              specify connections to make in the cosimulation
  --match_target_endpoints    set to true to enable connection of unconnected target endpoints
  --match_multiple            set to true to enable matching of multiple connections (default false)
  --always_check_regex        set to true to enable regex matching even if other matches are defined
```

The full CLI list is shown below including helics connection options and general options.

```text
Common options for all Helics Apps
Usage: [HELICS_APP] [OPTIONS] [input]

Positionals:
  input TEXT:FILE             The primary input file

Options:
  -h,-?,--help                Print this help message and exit
  --config-file,--config [helics_config.toml]
                              specify base configuration file
  --version                   Display program version information and exit
  --local                     Specify otherwise unspecified endpoints and publications as local (i.e. the names will be prepended with the player name)
  --stop TIME                 The time to stop the app
  --input TEXT:FILE           The primary input file
[Option Group: quiet]
  Options:
    --quiet                     silence most print output
[Option Group: Subcommands]
  Federate Info Parsing
  Positionals:
    config [helicsConfig.ini]   specify a configuration file
  Options:
    --version                   Display program version information and exit
    --config-file,--config [helicsConfig.ini]
                                specify a configuration file
    --config_section TEXT       specify the section of the config file to use
    --config_index INT          specify the section index of the config file to use for configuration arrays
    -n,--name TEXT              name of the federate
    --corename TEXT             the name of the core to create or find
    -i,--coreinitstring TEXT (Env:HELICS_CORE_INIT_STRING)
                                The initialization arguments for the core
    --brokerinitstring TEXT     The initialization arguments for the broker if autogenerated
    --broker,--brokeraddress TEXT
                                address or name of the broker to connect
    --brokerport INT:POSITIVE   Port number of the Broker
    --port INT:POSITIVE         Specify the port number to use
    --localport TEXT            Port number to use for connections to this federate
    --autobroker                tell the core to automatically generate a broker if needed
    --debugging                 tell the core to allow user debugging in a nicer fashion
    --observer                  tell the federate/core that this federate is an observer
    --allow_remote_control,--disable_remote_control{false}
                                enable the federate to respond to certain remote operations such as disconnect
    --json                      tell the core and federate to use JSON based serialization for all messages, to ensure compatibility
    --profiler TEXT [log]       Enable profiling and specify a file name (NOTE: use --profiler_append=<filename> in the core init string to append to an existing file)
    --broker_key,--brokerkey,--brokerKey TEXT
                                specify a key to use to match a broker should match the broker key
    --offset TIME               the offset of the time steps (default in ms)
    --period TIME               the execution cycle of the federate (default in ms)
    --stoptime TIME             the maximum simulation time of a federate (default in ms)
    --timedelta TIME            The minimum time between time grants for a Federate (default in ms)
    --inputdelay TIME           the INPUT delay on incoming communication of the federate (default in ms)
    --outputdelay TIME          the output delay for outgoing communication of the federate (default in ms)
    --grant_timeout TIME        timeout to trigger diagnostic action when a federate time grant is not available within the timeout period (default in ms)
    --maxiterations INT:POSITIVE
                                the maximum number of iterations a federate is allowed to take
    --loglevel INT:{summary,none,connections,no_print,profiling,interfaces,error,timing,warning,data,debug,trace}:value in {summary->6,none->-4,connections->9,no_print->-4,profiling->2,interfaces->12,error->0,timing->15,warning->3,data->18,debug->21,trace->24} OR {6,-4,9,-4,2,12,0,15,3,18,21,24} (Env:HELICS_LOG_LEVEL)
                                the logging level of a federate
    --separator CHAR [/]        separator character for local federates
    -f,--flags,--flag ...       named flag for the federate
  [Option Group: quiet]
    Options:
      --quiet                     silence most print output
  [Option Group: network type]
    Options:
      --core TEXT [()]            type or name of the core to connect to
      --force_new_core            if set to true will force the federate to generate a new core
      -t,--coretype TEXT [()]  (Env:HELICS_CORE_TYPE)
                                  type  of the core to connect to
  [Option Group: encryption]
    options related to encryption
    Options:
      --encrypted (Env:HELICS_ENCRYPTION)
                                  enable encryption on the network
      --encryption_config TEXT (Env:HELICS_ENCRYPTION_CONFIG)
                                  set the configuration file for encryption options
  [Option Group: realtime]
    Options:
      --rtlag TIME                the amount of the time the federate is allowed to lag realtime before corrective action is taken (default in ms)
      --rtlead TIME               the amount of the time the federate is allowed to lead realtime before corrective action is taken (default in ms)
      --rttolerance TIME          the time tolerance of the real time mode (default in ms)
Command line options for the Connector App
Usage: [OPTIONS]

Options:
  --version                   Display program version information and exit
  --connection [INTERFACE1,INTERFACE2,DIRECTIONALITY,TXT...] ...
                              specify connections to make in the cosimulation
  --match_target_endpoints    set to true to enable connection of unconnected target endpoints
  --match_multiple            set to true to enable matching of multiple connections (default false)
  --always_check_regex        set to true to enable regex matching even if other matches are defined
```

also permissible are all arguments allowed for federates and any specific broker specified:

[Command line reference](cmdArgs.md)

## Federate Support

The federate object has support for linking with connector operation. Interface definitions placed in a `"potential_interfaces"` object in a json configuration file will activate the potential interfaces sequence automatically. And based on the response of the connector will automatically create the interfaces defined if they are used. The interface objects can later be retrieved through normal means. No additional sequence or callbacks is needed on the federate. Json configuration is currently the only means to trigger this feature. The definitions for the interfaces in the potential interfaces are exactly the same as normally defining an interface in json.

### interface Templates

The federate object and the connector also support interface definition templates, for example

```json
{
  "potential_interfaces": {
    "endpoint_templates": [
      {
        "name": "obj${number}/ept${letter}/type${letter}/mode${letter}",
        "number": ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0"],
        "letter": [
          "A",
          "B",
          "C",
          "D",
          "E",
          "F",
          "G",
          "H",
          "I",
          "J",
          "K",
          "L",
          "M",
          "N",
          "O",
          "P",
          "Q",
          "R",
          "S",
          "T",
          "U",
          "V",
          "W",
          "X",
          "Y",
          "Z"
        ],
        "template": { "global": true }
      }
    ]
  }
}
```

or

```json
{
  "potential_interfaces": {
    "publications": [
      { "name": "pub1", "global": true, "type": "double" },
      { "name": "pub2", "global": true, "type": "double" }
    ],
    "inputs": [
      { "name": "inp2", "global": true, "type": "double" },
      { "name": "inp1", "global": true, "type": "double" }
    ],
    "publication_templates": [
      {
        "name": "${field1}/${field2}",
        "field1": ["obj1", "obj2", "obj3"],
        "field2": [
          ["type1", "double"],
          ["type2", "int"],
          ["type3", "double"]
        ],
        "template": { "global": true }
      }
    ],
    "input_templates": [
      {
        "name": "${field1}/${field2}",
        "field1": ["objA", "objB", "objC"],
        "field2": [
          ["typeA", "double", "W"],
          ["typeB", "int"],
          ["typeC", "double", "kV"]
        ],
        "template": { "global": true }
      }
    ]
  }
}
```

The connector will evaluate all possibilities for the template for possible connections. The field name in `${fieldName}` is searched for in the json file. They may be duplicated, but are treated as independent for evaluation purposes as in the first example. The first template example defines over 175,000 different possible interfaces. The type of the interface can be defined as part of the template name, with the particular name as the key.
