# helics_cli


Once the communication structure between federates has been configured and the timing architecture established, the co-simulation can be launched. This will create the federates as entities recognized by the broker, set up the communication channels for their messages to be passed, pass some initial messages, and execute some preliminary code as preparation for the beginning of the co-simulation proper. The last step is particularly important if the federates need to reach a self-consistent state as an initial condition of the system.

Execution of the co-simulation is done with `helics-cli`, which condenses the commands to launch each federate into a single executable. Details for how to launch a HELICS co-simulation can be found on the [`helics-cli`](./helics_cli.md) page.

This section will describe the uses of helics_cli and how to configure it for configuring federates and launching co-simulations.

## Example from Base Model

show what each component of the runner file is doing -- it's just telling helics to create a broker, and execute each federate