# helics_cli

Execution of the HELICS co-simulation is done from the command line with `helics_cli`, or Command Line Interface. Each simulator must be executed individually in order to join the federation. `helics_cli` condenses these command line execution commands into one executable, called the runner file.

All the [examples](../examples/examples_index.md) are written with a runner file for execution. In the [Fundamental Base Example](../examples/fundamental_examples/fundamental_examples_index.md), we need to launch three things: the broker, the Battery federate, and the Charger federate.

The file `fundamental_default_runner.json` includes:

```
{
  "federates": [
    {
      "directory": ".",
      "exec": "helics_broker -f 2 --loglevel=7",
      "host": "localhost",
      "name": "broker"
    },
    {
      "directory": ".",
      "exec": "python -u Charger.py 1",
      "host": "localhost",
      "name": "Charger"
    },
    {
      "directory": ".",
      "exec": "python -u Battery.py 1",
      "host": "localhost",
      "name": "Battery"
    }
  ],
  "name": "fundamental_default"
}
```

This tells `helics_cli` to launch three federates named `broker`, `Charger`, and `Battery`. The `directory` tells HELICS the location of the executable. For the `broker`, the executable `helics_broker` should be [configured to suite your needs](../installation/index.md). The `broker` is launched with the executable `helics_broker`, to which we pass the information `-f 2` meaning there will be two federates, and `--loglevel=7` meaning that we want [all internal messages](./logging.md) to be sent to the log file.

The other two federates are Python based. We instruct `helics_cli` to launch these with the `exec` command `python -u`.

Once the runner file is specified to include information about where the executables live (`directory`), the execution command for each (`exec`), the host, and the name, the entire federation can be launched with the following command:

```
> helics run --path=fundamental_default_runner.json
```

The next section discusses using the Web Interface to interact with a running HELICS co-simulation. The federation must be launched with `helics_cli` in order to use the Web Interface.
