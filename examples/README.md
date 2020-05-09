# HELICS Examples

## More Examples

Please see the [HELICS Examples Repo](https://github.com/GMLC-TDC/HELICS-Examples) for many more examples than are available in the HELICS repository
This includes examples in many of the interface languages.

Good places to start include:

[C Example](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/c)
[Python Example](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/python/pi-exchange)
[C++ Example](https://github.com/GMLC-TDC/HELICS-Examples/blob/master/cpp/valueFederate1/valueFed.cpp)
[Matlab Example](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/matlab/pi-exchange)

### Running a simple player recorder example

1.  Open three terminal windows
2.  Run the following in the first window

```bash
cd /path/to/helics_install/bin
./helics_broker 2
```

3.  Run the following in the second window

```bash
cd /path/to/helics_install/bin
./helics_player /path/to/HELICS/examples/example1.player
```

4.  Run the following in the last window

```bash
cd /path/to/helics_install/bin
./helics_recorder /path/to/HELICS/examples/example1.recorder -o output.log
```

This will run a 24 second (simulation time) co-simulation between a "player" federate that reads data from a file and a "recorder" federate that records selected data to a file (output.log).
