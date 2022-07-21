# Running HELICS Co-Simulations

Execution of the HELICS co-simulation is done from the command line using the `helics run ...` command in [PyHELICS](https://github.com/GMLC-TDC/pyhelics). Each simulator must be executed individually in order to join the federation. The `helics run ...` command condenses these individual command line execution commands through the use of a JSON called the "runner file"".

All the [examples](../examples/examples_index.md) are written with a runner file for execution. In the [Fundamental Base Example](../examples/fundamental_examples/fundamental_examples_index.md), we need to launch three things: the broker, the Battery federate, and the Charger federate.

The file `fundamental_default_runner.json` includes:

```json
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

This tells HELICS to launch three federates named `broker`, `Charger`, and `Battery`. The `directory` tells HELICS the location of the executable. For the `broker`, the executable `helics_broker` should be [configured to suite your needs](../installation/index.md). The `broker` is launched with the executable `helics_broker`, to which we pass the information `-f 2` meaning there will be two federates, and `--loglevel=7` meaning that we want [all internal messages](./logging.md) to be sent to the log file.

The other two federates are Python based and just need to be called with `python -u`.

Once the runner file is specified to include information about where the executables live (`directory`), the execution command for each (`exec`), the host, and the name, the entire federation can be launched with the following command:

```shell
> helics run --path=fundamental_default_runner.json
```

The next section discusses using the Web Interface to interact with a running HELICS co-simulation. The federation must be launched in this ways in order to use the Web Interface.
