# FMUs with HELICS

```{eval-rst}
.. toctree::
    :maxdepth: 1


```

This example shows how to use functional mock-up units (FMUs) with a HELICS-based co-simulation.

- [Where is the code?](#where-is-the-code)
- [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
- [HELICS components](#helics-components)
- [Building the FMU](#building-the-fmu)
- [Configuring the co-simulation](#setting-up-the-co-simulation)
- [Execution and Results](#execution-and-results)

## Where is the code?

The code for the [Advanced examples](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced) can be found in the HELICS-Examples repository on GitHub. This example on [using an FMU in a HELICS-based co-simulation](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_fmu). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).


## What is this co-simulation doing?

This example shows you how to take an existing FMU and incorporate it into a HELICS-based co-simulation. This effectively allows FMUs to act as HELICS federates.  

The [Functional Mock-up Interface (FMI)](https://fmi-standard.org/) is a modeling language popularly implemented by the commercial tool [Modelon](https://modelon.com/) (_nee_ Modelica) but also has an open-source implementation (OpenModelica)[https://openmodelica.org/]. The modeling language allows for a black-box description of a model's interfaces to support co-simulation of various modeled entities through the creation of an FMU. The FMU, when used for co-simulation, consists of two main components:
	1. An XML file describing the data exchange interfaces
	2. Binary version of the model with callable FMI-defined functions that simulate the model.
	
FMI effectively defines another means of performing a co-simulation using FMUs and the HELICS team has created a means by which these FMUs can join a HELICS-based co-simulation.


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
	- `set` - Allow for interaction with the interface values defined by the FMU. In this case it is used to define initalization values.


## Execution and Results
To execute this example, use the provided runner file:

```
helics run --path=runner.json
```

The FMU has been designed to replicate the model used in this example suite and the results should be identical to those provided in the fundamental default example:

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultcharger.png)


## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](mailto:helicsteam@helics.org)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
