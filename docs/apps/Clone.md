# Clone

The Clone application is one of the HELICS apps available with the library
Its purpose is to provide a easy way to clone a federate for later playback
It acts as a federate that can "capture" values or messages from a single federate
It also captures the interfaces and subscriptions of a federate and will store
those in a configuration file that can be used by the [Player](Player.md).
The clone app will try to match the federate being cloned as close as possible
in timing of messages and publications and subscriptions. At present it does
not match nameless publications or filters.

## Command line arguments

```text
Helics Clone App
Usage: helics_app clone [OPTIONS]

Command line options for the Clone App
Usage: [OPTIONS] [capture]

Positionals:
  capture TEXT                name of the federate to clone

Options:
  --allow_iteration           allow iteration on values
  -o,--output TEXT=clone.json the output file for recording the data

Options:
  -h,-?,--help                Print this help message and exit

```

also permissible are all arguments allowed for federates and any specific broker specified:

[Command line reference](cmdArgs.html)

the clone app is accessible through the helics_app

```bash
helics_app clone fed1 -o fed1.json -stop 10
```

### output

The Clone app captures output and configuration in a JSON format the [Player](Player) can read.
All publications of a federate are created as global with the name of the original federate, so a player could be named something
else if desired and not impact the transmission.
