# Default Advanced Example

The Advanced Base example walks through a HELICS co-simulation between three python federates, one of each type: value federate (`Battery.py`), message federate (`Controller.py`), and combination federate (`Charger.py`). This serves as the starting point for many of the other advanced examples and is an extension of the [Base Example.](../fundamental_examples/fundamental_default.md)

The Advanced Base Example tutorial is organized as follows:

- [Example files](#example-files)
- [Co-simulation Setup](#co-simulation-setup)
  - [Messages + Values](#messages-and-values)
  - [Co-simulation Execution and Results](#co-simulation-execution-and-results)
- [Questions and Help](#questions-and-help)

## Example files

All files necessary to run the Advanced Base Example can be found in the [Advanced Examples repository:](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_default)

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_examples_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_default)

The files include:

- Python program and configuration JSON for Battery federate
- Python program and configuration JSON for Charger federate
- Python program and configuration JSON for Controller federate
- HELICS runner JSON to enable execution of the co-simulation

## Co-simulation Setup

### Messages and Values

As you may or may not have read in the [User Guide](../../fundamental_topics/message_federates.md), one of the key differences between value exchange and the message exchange is that value exchange paths are defined once the federation has been initialized but message exchanges are dynamic and can travel from any endpoint to any endpoint throughout the co-simulation. The diagram below shows the three federates used in this example with the representative interfaces for both the value and message exchanges.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_signal_topology.png)

### Co-simulation Execution and Results

As in the [Fundamental Base Example](../fundamental_examples/fundamental_default.md), the HELICS runner is used to launch the co-simulation:

```shell
> helics run --path=advanced_default_runner.json
```

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_battery_SOCs.png)

This is the view of each battery as it is charged and two things are immediately obvious:

1. The impact of the charging level is pronounced. The first Batt1 takes almost half the simulation to charge but when its replacement is placed on the charger, it starts at a similar SOC but charges in a fraction of the time. The impact of the charging power supported by each EV is significant.
2. Most of the batteries fail to reach 100% SOC, some dramatically so. This is due to the current measurement error leading to a mis-estimate of SOC and thus premature termination of the charging. This can be seen the following graph

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_estimated_SOCs.png)

As previously mentioned, the current measurement noise is a function of the total magnitude of the current and thus as the battery charges up and the current draw drops, the noise in the measurement becomes a bigger fraction of the overall value. This results in the noisiest SOC estimates at higher SOC values. This is clearly seen in the EV1 value that starts the co-simulation relatively smooth and steadily increases in noisiness.

This graph also clearly shows that each EV was estimated to have a 100% SOC when the charging was terminated even though we know from the previous graph that full charge had not been reached.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_charging_power.png)

The data shown in the power graph is arguably the point of the analysis. It shows our maximum charging power for this simulated time as 80 kW. If this is the only simulation result we have, we would be inclined to use this as a design value for our electricity delivery infrastructure. More nuanced views could be had, though, by:

1. Running this co-simulation multiple times using a different random seed to see if 80 kW is truly the maximum power draw. (We do a version of this in an [example demonstrating how to run multiple HELICS co-simulations simultaneously](./advanced_brokers_simultaneous.md) on a single compute node.)
2. Plotting the charging power as a histogram to get a better understanding of the distribution of the instantaneous charging power. (We also do this as part of our [example on using an orchestration tool to use HELICS co-simulations as part of a more complex analysis](./advanced_orchestration.md).)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
