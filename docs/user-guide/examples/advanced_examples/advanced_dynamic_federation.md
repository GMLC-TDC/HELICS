# Dynamic Federations

This example demonstrates the capability of HELICS to allow federates to join the federation after simulation time zero has been granted and to leave the federation before other federates complete.

- [Where is the code?](#where-is-the-code)
- [What is this co-simulation doing?](#what-is-this-co-simulation-doing)
  - [Differences compared to the Advanced Default example](#differences-compared-to-the-advanced-default-example)
  - [HELICS differences](#helics-differences)
  - [Implementation](#implementation)
- [Execution and Results](#execution-and-results)
- [Questions and Help](#questions-and-help)

## Where is the code?

The example code for showing the operation of a dynamic federation [can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_dynamic_federation). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

## What is this co-simulation doing?

HELICS can allow federates to join the federation late (after others have already entered executing mode and been granted simulation time zero) and leave early. This example demonstrates that functionality. In this case, EV 5 joins the co-simulation late and leaves the co-simulation early.

### Differences compared to the Advanced Default example

Compared to the Advanced Default example, this example has EV 5 joining the federation late and leaving the federation early.

### HELICS differences

HELICS has always allowed a federate to leave a federation early. That is, it has never been a requirement that all federates end their part of the co-simulation at the same simulated time. In most cases, federates will leave the co-simulation at the same or nearly the same simulated time so the model retains consistency.

The ability for a federate to join a co-simulation late was first fully supported in HELICS v3.4.0. Joining a federation late is more disruptive to a co-simulation because HELICS must incorporate any new publications, subscriptions, and endpoints into the dependency graph that it uses to determine which federates should be granted time. There is a computational cost to having federates join the federation and recalculate the dependency graph, so there can be a non-trivial performance cost depending on the details of the federation.

To enable full dynamic co-simulation, `helics_broker` needs to be started with the command-line `--dynamic` switch. For this example, the full command line looks like:

```shell
helics_broker -f3 --dynamic
```

### Implementation

`Battery.py` has been modified to model both the EVs that join the federation at the beginning of the co-simulation and EV 5, which joins late. The `--late` flag is set for the latter. To help with data management and message passing, the variable `not_charging_value` is used as a semaphore to indicate that EV 5 is not attached to its charger, preventing `Charger.py` from misbehaving while EV 5 is not part of the co-simulation.

Ideally, a separate HELICS publication and subscription would be used to indicate whether the EV was present instead of using a semaphore. This would require enough rework in this example, and likely in the other examples to keep the suite consistent, that we decided not to undertake it. For more serious modeling efforts, adding such a signal would simplify the existing logic for Battery and Charger and make the code easier to understand and maintain.

## Execution and Results

Run the co-simulation:

```shell
helics run --path=./dynamic_federation_runner.json
```

With EV 5 not being present for the full co-simulation, the resulting graphs differ from those in the [Advanced Default example](./advanced_default.md). Note that the x-axis showing the result of just the EV 5 SOC does not cover the same period of time as the graph showing EVs 1-4.

Also worth noting are the blips in the EV 5 SOC as shown in the bottom graph. This is not unexpected and is likely something that could be mitigated in federate code. If you're running a dynamic federation, managing the HELICS data exchanges at these transition points, as well as before the federate joins and after the federate leaves, will take extra consideration and effort.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_dynamic_federation_1.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_dynamic_federation_2.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_dynamic_federation_3.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_dynamic_federation_4.png)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
