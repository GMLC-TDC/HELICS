# Tracer

The Tracer application is one of the HELICS apps available with the library
Its purpose is to provide a easy way to display data from a federation
It acts as a federate that can "capture" values or messages from specific publications
or direct endpoints or cloned endpoints which exist elsewhere and either trigger callbacks or display it to a screen
The main use is a simple visual indicator and a monitoring app

## Command line arguments

```text
allowed options:

command line only:
  -? [ --help ]          produce help message
  -v [ --version ]       display a version string
  --config-file arg      specify a configuration file to use

configuration:
  --stop arg             the time to stop recording
  --tags arg             tags to record, this argument may be specified any
                         number of times
  --endpoints arg        endpoints to capture, this argument may be specified
                         multiple time
  --sourceclone arg      existing endpoints to capture generated packets from,
                         this argument may be specified multiple time
  --destclone arg        existing endpoints to capture all packets with the
                         specified endpoint as a destination, this argument may
                         be specified multiple time
  --clone arg            existing endpoints to clone all packets to and from
  --capture arg          capture all the publications of a particular federate
                         capture="fed1;fed2"  supports multiple arguments or a
                         semicolon/comma separated list
  -o [ --output ] arg    the output file for recording the data
  --mapfile arg          write progress to a memory mapped file


federate configuration
  -b [ --broker ] arg    address of the broker to connect
  -n [ --name ] arg      name of the player federate
  --corename arg         the name of the core to create or find
  -c [ --core ] arg      type of the core to connect to
  --offset arg           the offset of the time steps
  --period arg           the period of the federate
  --timedelta arg        the time delta of the federate
  -i [ --coreinit ] arg  the core initialization string
  --inputdelay arg       the input delay on incoming communication of the
                         federate
  --outputdelay arg      the output delay for outgoing communication of the
                         federate
  -f [ --flags ] arg     named flags for the federate

```

also permissible are all arguments allowed for federates and any specific broker specified:

[Command line reference](cmdArgs.html)

the tracer executable also takes an untagged argument of a file name for example

```bash
helics_app tracer tracer_file.txt --stop 5
```

Tracers support both delimited text files and JSON files some examples can be found in, they are otherwise the same as options for recorders.

[Tracer configuration examples](https://github.com/GMLC-TDC/HELICS/tree/master/tests/helics/apps/test_files)

## Config File Detail

### subscriptions

a simple example of a recorder file specifying some subscriptions

```text
#FederateName topic1

sub pub1
subscription pub2
```

`#` signifies a comment

if only a single column is specified it is assumed to be a subscription

for two column rows the second is the identifier
arguments with spaces should be enclosed in quotes

```eval_rst
+------------------------------+------------------------------------------------------------+
| interface                    | description                                                |
+==============================+============================================================+
| s, sub, subscription         | subscribe to a particular publication                      |
+------------------------------+------------------------------------------------------------+
| endpoint, ept, e             | generate an endpoint to capture all targeted packets       |
+------------------------------+------------------------------------------------------------+
| source, sourceclone,src      | capture all messages coming from a particular endpoint     |
+------------------------------+------------------------------------------------------------+
| dest, destination, destclone | capture all message going to a particular endpoint         |
+------------------------------+------------------------------------------------------------+
| capture                      | capture all data coming from a particular federate         |
+------------------------------+------------------------------------------------------------+
| clone                        | capture all message going from or to a particular endpoint |
+------------------------------+------------------------------------------------------------+
```

for 3 column rows the first must be either clone or capture
for clone the second can be either source or destination and the third the endpoint name
\[for capture it can be either "endpoints" or "subscriptions"\]

### JSON configuration

Tracers can also be specified via JSON files

here are two examples of the text format and equivalent JSON

```text
#list publications and endpoints for a recorder

pub1
pub2
e src1
```

JSON example

```json
{
  "subscriptions": [
    {
      "key": "pub1",
      "type": "double"
    },
    {
      "key": "pub2",
      "type": "double"
    }
  ],
  "endpoints": [
    {
      "name": "src1",
      "global": true
    }
  ]
}
```

some configuration can also be done through JSON through elements of "stop","local","separator","timeunits"
and file elements can be used to load up additional files
