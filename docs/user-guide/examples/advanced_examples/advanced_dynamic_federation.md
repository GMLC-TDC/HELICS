# Dynamic Federations

This example demonstrates the capability of HELICS to allow federates to join the federation after simulation time zero has been granted as well as to leave the federation prior to other federates completing.  

- [Dynamic Federations](#dynamic-federations)
  - [Where is the code?](#where-is-the-code)
  - [What is this co-simulation doing?](#what-is-this-co-simulation-doing)
    - [Differences compared to the Advanced Default example](#differences-compared-to-the-advanced-default-example)
      - [HELICS Differences](#helics-differences)
    - [Implementation](#implementation)
  - [Execution and Results](#execution-and-results)
  - [Questions and Help](#questions-and-help)

## Where is the code?

This example code on [showing the operation of a dynamic federation](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_dyanmic_federation). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_connector_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_connector)

## What is this co-simulation doing?

HELICS has the ability to allow federates to join the federation late (after others have already entered executing mode and been gratned simulation time zero) and leave early. This example demonstrates this functionality. In this case, EV 5 joins the co-simulation late and leaves the co-simulation early. 

### Differences compared to the Advanced Default example

As compared to the Advanced Default example, this example has EV 5 joining the federation late and leaving the federation early.

#### HELICS Differences

HELICS has always had the ability for a federate to leave a federation early. That is, it has never been a requirement that all federates end their part of the co-simatulation at the same simulated time. In most cases, federates will end up leaving the co-simulation at the same or nearly same simulated time so that the model retains consistency. 

The ability for a federate to join a co-simulation late was first fully supported in HELICS v3.4.0. Joining a federation late is much more disruptive to a co-simulation as it requires HELICS to incorporate any new publications, subscriptions, and endpoints into the federate depenedency graph that it uses to figure out who should be granted time when. That is, there is a computational cost to having federates join the federation and recalculate the dependency graph and thus there could be a non-trivial performance cost but it entirely depends on the details of the federation under consideration.

To enable full dynamic co-simulation, `helics_broker` needs to be started with the command-line `--dynamic` switch. For this example, the full command line looks like:

```shell
$ helics_broker -f3 --dynamic
```
 
 ### Implementation
 "Battery.py" has been modified to allow it to model both the EVs that join the federation at the beginning of the co-simulation and EV 5 which joins late; the `--late` flag is set for the later. To help with the data management and message passing, the variable `not_charging_value` is used as a semaphore to indicate that EV 5 is not attached to its charger, preventing Charging.py from mis-behaving while EV 5 is not part of the co-simulation. Ideally, rather than using a semaphore like this, a separate HELICS publication and subscription would be used to indicate whether the EV was present. This would require enough re-work in this example (and likely all the others to keep the suite relatively consistent) that we decided not to undertake it. For more serious modeling efforts, adding such a signal would simplify the existing logic for Battery and Charger and make the code easier to understand and more maintainable.


## Execution and Results

Run the co-simulation:

```shell
$ helics run --path=./dynamic_federation_runner.json
```

With EV 5 not being present, the resulting graphs differ from those in the [Advanced Default example](./advanced_default.md). Note that the x-axis showing the result of just the EV 5 SOC does not cover the same period of time as the graph showing EVs 1-4. 

Also worth noting are the blips in the EV 5 SOC as shown in the bottom graph. This is not unexpected and is likely something that could be mitigated in federate code (but clearly has not been.) If you're running a dynamic federation, managing the HELICS data exchanges at these transition points (as well as before the federate joins and after the federate leaves) will take extra consideration and effort.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_dynamic_federation_1.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_dynamic_federation_2.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_dynamic_federation_3.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_dynamic_federation_4.png)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
