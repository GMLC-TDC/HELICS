# Fundamental Topics

```eval_rst
.. toctree::
    :hidden:
    :maxdepth: 2

    co-simulation_overview
    helics_key_concepts
    helics_co-sim_sequence
	

```


- [**Co-Simulation Overview**](./co-simulation_overview.md) - A more detailed discussion of what co-simulation is and how it is used
<!--- suggest renaming 'key concepts' md to 'terminology' -->
- [**HELICS Terminology**](./helics_key_concepts) - Key terms and concepts to understand before running co-simulations with HELICS
- [**HELICS Sequence of Components**](./helics_co-sim_sequence.md) - A notional walk-through of a simple transmission and distribution HELICS co-simulation to show the basic steps the software runs through
	- [**(Design) Integrating a Simulator with HELICS**](./simulator_integration.md) - How to connect an existing simulator with HELICS
	- [**Federate Configuration**](./federates.md) - Discussion of the different types of federates in HELICS and how to configure them.
		- [**Value Federates**](./value_federates.md)
		- [**Message Federates**](./message_federates.md)
		- [**Message Filters**](./filters) - How HELICS message filters can be implemented natively in HELICS or as stand-alone federates
	- [**Timing**](./timing.md) - How HELICS coordinates the simulation time of all the federates in the federation

	- [**(Launch) Execution with `helics_cli`**](./helics_cli.md) - The HELICS team has developed `helics_cli` as a standardized means of running HELICS co-simulations.
	- **Simulation**	
		- [**Connecting to a Running Simulation**](./webserver.md) - Getting live information from a running co-simulation through a webserver.
		- [**Using the web interface**](./web_interface.md) - Using the webserver, HELICS also has a built-in web interface for running, monitoring, and diagnosing co-simulations.
	- [**(Termination) Logging**](./logging.md) - Discussion of logging within HELICS and how to control it.

