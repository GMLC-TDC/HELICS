# Source

The Source app generates signals for other federates, it functions similarly to the
player but doesn't take a prescribed file instead it generates signals according to some
mathematical function, like sine, ramp, pulse, or random walk.
This can be useful for sending probing signals or just testing responses of the federate to various stimuli.

## Command line arguments

```text
allowed options:

command line only:
  -? [ --help ]          produce help message
  -v [ --version ]       display a version string
  --config-file arg      specify a configuration file to use

configuration:
  --datatype arg         type of the publication data type to use
  --local                specify otherwise unspecified endpoints and
                         publications as local( i.e.the keys will be prepended
                         with the player name
  --separator arg        specify the separator for local publications and
                         endpoints
  --time_units arg        the default units on the timestamps used in file based
                         input
  --stop arg             the time to stop the player

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

the player executable also takes an untagged argument of a file name for example

```bash
helics_player player_file.txt --stop 5
```

Players support both delimited text files and JSON files some examples can be found in

[Player configuration examples](https://github.com/GMLC-TDC/HELICS/tree/master/tests/helics/apps/test_files)

## Config File Detail

### publications

a simple example of a player file publishing values

```csv
#second    topic                type(opt)                    value
-1.0, pub1, d, 0.3
1, pub1, 0.5
3, pub1 0.8
2, pub1 0.7
# pub 2
1, pub2, d, 0.4
2, pub2, 0.6
3, pub2, 0.9
```

`#` signifies a comment
the first column is time in seconds unless otherwise specified via the `--time_units` flag or other configuration means
the second column is publication name
the final column is the value
the optional third column specifies a type valid types are

time specifications are typically numerical with optional units
`5` or `"500 ms"` or `23.7us` if there is a space between the number and units it must be enclosed in quotes
if no units are specified the time defaults to units specified via `--time_units` or seconds if none were specified
valid units are "s", "ms", "us", "min", "day", "hr", "ns", "ps" the default precision in HELICS is ns so time specified in ps is not guaranteed to be precise

```eval_rst
+--------------------+---------------------------+-----------------------+
| identifier         | type                      | Example               |
+====================+===========================+=======================+
| d,f, double        | double                    | 45.1                  |
+--------------------+---------------------------+-----------------------+
| s,string           | string                    | "this is a test"      |
+--------------------+---------------------------+-----------------------+
| i, i64, int        | integer                   | 456                   |
+--------------------+---------------------------+-----------------------+
| c, complex         | complex                   | 23+2j, -23.1j, 1+3i   |
+--------------------+---------------------------+-----------------------+
| v, vector          | vector of doubles         | [23.1,34,17.2,-5.6]   |
+--------------------+---------------------------+-----------------------+
| cv, complex_vector | vector of complex numbers | [23+2j, -23.1j, 1+3i] |
+--------------------+---------------------------+-----------------------+
```

capitalization does not matter

values with times <0 are sent during the initialization phase
values with time==0 are sent immediately after entering execution phase

### Messages

messages are specified in one of two forms

```text
m <time> <source> <dest>  <data>
```

or

```text
m <sendtime> <deliverytime> <source> <dest> <time> <data>
```

the second option allows sending events at a different time than they are triggered

### JSON configuration

player values can also be specified via JSON files

here are two examples of the text format and equivalent JSON

```text
#example player file
mess 1.0 src dest "this is a test message"
mess 1.0 2.0 src dest "this is test message2"
M 2.0 3.0 src dest "this is message 3"
```

JSON example

```json
{
  "messages": [
    {
      "source": "src",
      "dest": "dest",
      "time": 1.0,
      "data": "this is a test message"
    },
    {
      "source": "src",
      "dest": "dest",
      "time": 2.0,
      "data": "this is test message2"
    },
    {
      "source": "src",
      "dest": "dest",
      "time": 3.0,
      "data": "this is message 3"
    }
  ]
}
```

```text
#second    topic                type(opt)                    value
-1 pub1 d 0.3
1 pub1 d 0.5
2 pub1 d 0.7
3 pub1 d 0.8
1 pub2 d 0.4
2 pub2 d 0.6
3 pub2 d 0.9
```

Example JSON

```json
{
  "points": [
    {
      "key": "pub1",
      "type": "double",
      "value": 0.3,
      "time": -1
    },
    {
      "key": "pub2",
      "type": "double",
      "value": 0.4,
      "time": 1.0
    },
    {
      "key": "pub1",
      "value": 0.5,
      "time": 1.0
    },
    {
      "key": "pub1",
      "value": 0.8,
      "time": 3.0
    },
    {
      "key": "pub1",
      "value": 0.7,
      "time": 2.0
    },
    {
      "key": "pub2",
      "value": 0.6,
      "time": 2.0
    },
    {
      "key": "pub2",
      "value": 0.9,
      "time": 3.0
    }
  ]
}
```

some configuration can also be done through JSON through elements of "stop","local","separator","time_units"
and file elements can be used to load up additional files
