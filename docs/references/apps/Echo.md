# Echo

The Echo application is one of the HELICS apps available with the library
Its purpose is to provide a easy way to generate an echo response to a message
Mainly for testing and demos

## Command line arguments

```text
allowed options:

command line only:
  -? [ --help ]          produce help message
  -v [ --version ]       display a version string
  --config-file arg      specify a configuration file to use

configuration:
  --local                specify otherwise unspecified endpoints and
                         publications as local( i.e.the keys will be prepended
                         with the echo name
  --stop arg             the time to stop the app


configuration:
  -b [ --broker ] arg    address of the broker to connect
  -n [ --name ] arg      name of the player federate
  --corename arg         the name of the core to create or find
  -c [ --core ] arg      type of the core to connect to
  --offset arg           the offset of the time steps
  --period arg           the period of the federate
  --timedelta arg        the time delta of the federate
  -i [ --coreinit ] arg  the core initialization string
  --separator arg        separator character for local federates
  --inputdelay arg       the input delay on incoming communication of the
                         federate
  --outputdelay arg      the output delay for outgoing communication of the
                         federate
  -f [ --flags ] arg     named flag for the federate


configuration:
  --delay arg            the delay with which the echo app will echo message



```

also permissible are all arguments allowed for federates and any specific broker specified:

[Command line reference](cmdArgs.html)

the echo executable also takes an untagged argument of a file name for example

```bash
helics_app echo echo_file.txt --stop 5
```

The Echo app supports JSON files some examples can be found in

[Echo configuration examples](https://github.com/GMLC-TDC/HELICS/tree/master/tests/helics/apps/test_files)

the main property of the echo app is the delay time which messages are echoed.
