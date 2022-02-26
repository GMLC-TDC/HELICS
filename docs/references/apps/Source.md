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

[Command line reference](cmdArgs.md)
