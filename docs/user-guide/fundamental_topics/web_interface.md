# HELICS Web Interface

Once a federate has been granted the ability to move forward to a specific time (the granted time), the federate will execute its simulation, calculating its new state, behavior, or control action to advance to that time. Prior to these calculations, it will receive any messages that have been sent to it by other federates and after simulating up to the granted time, may send out messages with new values other federates may need.

Using the webserver that HELICS provides to access data about a cosimulation (without having to using the [HELICS APIs](../../references/api-reference/index.md) yourself), HELICS also has a GUI via a web page that allows users to more easily run, monitor and debug a co-simulation.

## Launching Web Interface

The web interface requires the use of [helics_cli](helics_run.md) to run the co-simulation and is created by the following command:

```sh
$ helics_cli server
127.0.0.1:8000
```

The response from helics_cli is the address of the web interface that can be used to run the co-simulation that has helics_cli has been configured to run. Copy and paste that into a web browser to access the interface.

## Web Interface Overview

![Ex. 1a message topology](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/web_interface_overview.png)

1. **Load web interface configuration** - A configuration file is provided to allow the user to be more specific about which federates and publications are of interest. Particularly for federations with large numbers of federates and/or large numbers of publications, it may not be desirable to display all of that information in the web interface, especially if it isn't needed. A configuration file can be loaded that will allow the user to restrict which federates and publications are added to the respective tables. xxxxxxx - Do we need to advise users to limit the size of these tables for performance reasons. Or is this largely expected to be a convenience for them? At what point does size become a problem?
2. **Run co-simulation** - This button will launch the co-simulation via helics_cli and run it to completion. The values in the "Federate Configuration" and "Publication" fields will update periodically to reflect the current state of the federation. This allows for some degree of monitoring while the co-simulation runs.
3. **Stop co-simulation** - Stops the currently running co-simulation, likely because it is clear something has gone horribly wrong.
4. **Debug run to next time grant** - Just pressing the play-pause button will advance the entire federation to the next granted time (across the entire federation). The text field updates with the next time to be granted but also allows the user to provide the granted time to run to next.
5. **Federate State** - Table showing the federates in the federation, the last simulated time they were granted and the simulated time they have requested. This list can be filtered using the "Search" text box though regular expressions are not supported.
6. **Publication** - Listing of publications from the federation showing the key, sending federate, simulated time of publication, value, and whether the value was updated at the last simulated time granted.

## Web Interface Configuration File

Forthcoming description of how the web interface configuration file is formatted and how to edit it to get the right federates and publications to appear in the web interface tables.

## Using the Web Interface

### Normal, everyday runs

When you have a co-simulation that you've vetted and feel comfortable will run without difficulty (or at least you don't anticipate a need to debug it), running the co-simulation to completion from the interface is straight-forward:

1. (optional) Load in a configuration file - If you just want to look at a few federates and/or publications to track the progress of the co-simulation, make a web interface configuration file that does so and load it up.
2. Press the "Run" button (number 2 in the screenshot above).

The helics_cli will launch the co-simulation and the web interface will update every five seconds (or as specified in the configuration file). This periodically updating view is helpful if only to see where the co-simulation is at (simulation time-wise) but also helps confirm that message values are as expected. When the co-simulation is done, xxxxxxx (as message will appear? How are we indicating this?). All messages from the co-simulation (or a subset as specified in the helic_cli configuration) have been stored in an SQLite database; the database can be directly queried or the data can be exported to file in a number of formats (CSV, JSON) and post-process by a tool of your choice.

### Debugging runs

Just like writing code, it is not unusual for a co-simulation to not quite work write the first time it's constructed and the web interface provides tooling to help verify and trouble-shoot the construction co-simulation. Here's how the web interface can be used try to get a diagnostic view of the federation's operation.

1. (optional but highly recommended) Load in a configuration file - Limit the federates and publications to those of interest and most revealing of the state of the simulation.
2. Enter a simulated time of interest and press the "Run to next grant" button (number 4 in the above diagram)
3. Check the state of the federation - When the co-simulation stops at the indicated time, check that all federates of interest have been granted and are requesting expected times. Check the message table and verify that all messages of interest have published with reasonable/expected values.
4. As necessary, proceed to subsequent or later time steps to hunt down the particular problem of interest.
