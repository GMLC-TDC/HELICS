# Fundamental Topics

```eval_rst
.. toctree::

    :maxdepth: 1

    helics_terminology
    simulator_integration
    federates
    timing
    helics_cli
    web_interface
    logging
	

```

- [**HELICS Terminology**](./helics_terminology) - Key terms and concepts to understand before running co-simulations with HELICS
- [**(Design) Integrating a Simulator with HELICS**](./simulator_integration.md) - How to connect an existing simulator with HELICS
	- **With JSON config file**
	- **With HELICS APIs**
- [**Federate Configuration**](./federates.md) - Discussion of the different types of federates in HELICS and how to configure them.
	- [**Value Federates**](./value_federates.md)
	- [**Message Federates**](./message_federates.md)
	- [**Message Filters**](./filters) - How HELICS message filters can be implemented natively in HELICS or as stand-alone federates
- [**Timing**](./timing.md) - How HELICS coordinates the simulation time of all the federates in the federation
- [**(Launch) Execution with `helics_cli`**](./helics_cli.md) - The HELICS team has developed `helics_cli` as a standardized means of running HELICS co-simulations.
- [**Simulation Management**](./web_interface.md) - Using the webserver, HELICS also has a built-in web interface for running, monitoring, and diagnosing co-simulations.
- [**(Termination) Logging**](./logging.md) - Discussion of logging within HELICS and how to control it.

