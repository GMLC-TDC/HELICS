# FMUs with HELICS

```{eval-rst}
.. toctree::
    :maxdepth: 1


```

This demonstrates the use of HELICS-FMI to integrate an FMU into a HELICS-based co-simulation. [Functional Mock-Up Interface (FMI)](https://fmi-standard.org/) is an existing method of integrating models that is similar to HELICS is some ways. FMI provides a way to specify a model and with the help of a supporting tool, compile into a binary and provide a companion specification of the interface to that model. In some cases, a solver for the model is also distributed along with the model, effectively making it a highly specialized simulation tool. These elements can be bundled up into a distributable unit called a "functional mock-up unit" (FMU) that can be distributed without revealing the inner workings of a model. HELICS provides a way to allow one or more FMUs to participate in a HELICS co-simulation using the HELICS-FMI application.

- [Where is the code?](#where-is-the-code)
- [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
- [HELICS components](#helics-components)
- [Building the FMU](#building-the-fmu)
- [Configuring the co-simulation](#configuring-the-co-simulation)
- [Execution and Results](#execution-and-results)

## Where is the code?

The code for the [Advanced examples](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced) can be found in the HELICS-Examples repository on GitHub. This example on [using an FMU in a HELICS-based co-simulation](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_fmu). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

Note that this code contains a component (the FMU) that has been compiled for Windows and thus will only function on Windows. The FMU source code has been included and can be compiled into an FMU for other platforms with the appropriate tools.

## What is this co-simulation doing?

This example shows you how to take an existing FMU and incorporate it into a HELICS-based co-simulation. This effectively allows FMUs to act as HELICS federates.

The [Functional Mock-up Interface (FMI)](https://fmi-standard.org/) is a modeling language popularly implemented by the commercial tool [Modelon](https://modelon.com/) (_nee_ Modelica) but also has an open-source implementation [OpenModelica](https://openmodelica.org/). The modeling language allows for a black-box description of a model's interfaces to support co-simulation of various modeled entities through the creation of an FMU. The FMU, when used for co-simulation, consists of two main components: 1. An XML file describing the data exchange interfaces 2. Binary version of the model with callable FMI-defined functions that simulate the model.

FMI effectively defines another means of performing a co-simulation using FMUs and the HELICS team has created a means by which these FMUs can join a HELICS-based co-simulation. To integrate the FMU into the example, HELICS-FMI acts as a bridge that is able to execute the "SimpleBattery.fmu" FMU. HELICS-FMI takes care of executing the FMU using the inputs from the rest of the co-simulation and providing the outputs on its behalf.

## HELICS components

To enable an FMU to act as a HELICS federate, the "helics_fmi" application as been developed. This application has the ability to read an FMU, set-up the defined data-exchange interfaces as HELICS value interfaces, and call the necessary simulation functions for the FMU to push it forward in simulated time. This puts the HELICS in the role of "master algorithm" (to use the FMI parlance).

As a more specialty item, helics_fmi is not included as a part of the standard HELICS distribution and must be built from source. The source code can be found in the ["helics_fmi"](https://github.com/GMLC-TDC/HELICS-FMI) repository and uses a CMake build process like the main HELICS library. You can follow [the instructions here on building the main HELICS library](https://docs.helics.org/en/latest/user-guide/installation/build_from_source.html) but work from the helics_fmi repo.

## Building the FMU

Distributed with the example is the [source code for the FMU battery model](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_fmu/SimpleBattery.mo). This source code needs to be compiled into an FMU for use in this example. Provided with this example is the [FMU for Windows](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_fmu/SimpleBattery.fmu); if running this example on another platform the FMU will have to be compiled on that platform to produce a valid binary.

## Configuring the Co-Simulation

With helics_fmi and FMU built, all that remains is providing command line options for helics_fmi. Looking at the [runner JSON for this example](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_fmu/runner.json), the following options are used:

- `stoptime` - The simulated time in seconds for ending the FMU simulation
- `step` - Simulation step size, equivalent to `period` in HELICS configuration
- `name` - Federate name
- `flags` - Special runtime flags to address particular FMU needs
- `set` - Allow for interaction with the interface values defined by the FMU. In this case it is used to define initialization values.

## Execution and Results

To execute this example, use the provided runner file:

```shell
helics run --path=runner.json
```

The FMU has been designed to replicate the model used in this example suite and the results should be identical to those provided in the fundamental default example:

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultcharger.png)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
