# HELICS example

### Running a simple example

1. Open three terminal windows
2. Run the following in the first window

```bash
cd /path/to/helics_install/bin
./helics_broker 2
```

3. Run the following in the second window

```bash
cd /path/to/helics_install/bin
./helics_player /path/to/HELICS/examples/example1.player
```

4. Run the following in the last window

```bash
cd /path/to/helics_install/bin
./helics_recorder /path/to/HELICS/examples/example1.recorder -o output.log
```

This will run a 24 second (simulation time) co-simulation between a "player" federate that reads data from a file and a "recorder" federate that records selected data to a file (output.log).

