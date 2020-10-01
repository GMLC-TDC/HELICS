# HELICS Sequence Components

As a co-simulation is, in some sense, a simulation of simulations, there are two levels of configuration required: the configuration of the individual federates as if they were running on their own (identifying models to be used, defining the start and stop time of the simulation, defining how the results of the simulation should be stored, etc...) and the configuration of how each federate will connect to the co-simulation and how it will interact with the other federates in the co-simulation. One of the goals of a co-simulation platform like HELICS is to make the later easier and more efficient by providing a standardized method of configuration. To provide a better understanding of why certain types of information are required during configuration and the implications of making certain choices, this section presents the components to a HELICS co-simulation in sequence of operation and includes a stand-alone simple co-simulation example to demonstrate the implementation of these components. 

This section provides an overview of the components, each of which are linked to separate sections for more detail.

```eval_rst
.. toctree::
    :maxdepth: 1
    
    simulator_integration
    federates
    timing
    helics_cli
    simulate
    logging

```



## [**Co-simulation design and integration**](./simulator_integration.md)

As a user, it will be up to you to understand the assumptions, modeling techniques, and dynamics of the simulators you are going to be tying together via HELICS. Using that knowledge you'll have to define the message topology (who is passing what information to whom) and the broker topology (which federates are connected to which brokers). Message topology requires understanding the interactions of the system the simulators are trying to replicate and identifying the boundary conditions where they could exchange data. Broker topology is somewhat optional (you can run a co-simulation with just a single broker) but offers an increase in performance if it is possible to identify groups of federates that interact often with each other but rarely with the rest of the federation. In such cases, assigning that group of federates their own broker will remove the congestion their messages cause with the federation as a whole.

The user guide page on [**co-simulation design and integration**](./simulator_integration.md) walks through the integration of a federate into HELICS with reference to simple examples.



## [**Communication configuration between federates**](./federates.md)
Every federate (instance of a simulator) will require configuration of the way it will communicate (send messages/information) to other federates in the federation. For simulators that already have HELICS support, the configuration takes the form of a JSON (or TOML) file; bespoke simulators can be configured with the HELICS APIs in the code or via a JSON file. The essential information that HELICS configuration defines is:

   _Federate name_ - The unique name this federate will be known as throughout the federation. It is essential this name is unique so that HELICS messages can route properly.

   _Core type_ - The core manages interface between the federation and the federate; there are several messaging technologies supported by HELICS. All federates in the same federation must have the same core type. <!--- (this requirement will be relaxed in the future). -->

   _Outputs and Inputs_ - Output configuration contains a listing of messages, data types, and units being sent by the federate; input configuration does the same for values being received by the federate. If supported by the simulator, these values can be mapped to internal variables of the simulator from the configuration file. 

   _Endpoints_ - Endpoints are sending and receiving points for HELICS messages to and from message federates. They are declared and defined for each federate. 

   _Time step size_ - This value defines the resolution of the simulator to prevent HELICS from telling the simulator to step to a time of which it has no concept (e.g. trying to simulate the time of 1.5 seconds when the simulator has a resolution of one second). 

## [**HELICS Time**](./timing.md)

Once the initializing phase is complete, the co-simulation proper begins. Every federate, based on the system it is modeling and what information it needs to run that simulation, will request a simulation time from its core. This time request indicates the point in simulated time at which the federate knows it will have to execute commands so it can simulate some portion of the model or behavior in the system. For example, there may be a federate simulating a building and based on the dynamics of the system, it knows the indoor temperature will not appreciably change over the next five minutes. 

Based on the time requests and grants from all the connected federates, a core will determine the next time it can grant to a federate to guarantee none of the federates will be asked to simulate a point in time that occurs in the past. If the core is doing its job correctly, every federate will receive a time that is the same as or larger than the last time it was granted. HELICS does support a configuration and some other situations that allows a federate to break this rule, but this is a very special situation and would require all the federates to support this jumping back in time, or accept non-causality and some randomness in execution. 


## [**Launch**](./helics_cli.md)

Once the communication structure between federates has been configured and the timing architecture established, the co-simulation can be launched. This will create the federates as entities recognized by the broker, set up the communication channels for their messages to be passed, pass some initial messages, and execute some preliminary code as preparation for the beginning of the co-simulation proper. The last step is particularly important if the federates need to reach a self-consistent state as an initial condition of the system.
<!--- what is meant by "self-consistent state"? -->

Execution of the co-simulation is done with `helics-cli`, which condenses the commands to launch each federate into a single executable.

## [**Simulation**](./simulate.md)
 
Once a federate has been granted the ability to move forward to a specific time (the granted time), the federate will execute its simulation, calculating its new state, behavior, or control action to advance to that time. Prior to these calculations, it will receive any messages that have been sent to it by other federates and after simulating up to the granted time, may send out messages with new values other federates may need.



## [**Termination and Debugging**](./logging.md) 

After all possible time grants have been exhausted, the federates signal to their core that they are leaving the federation. Once all the federates have left, the rest of the infrastructure disassembles itself and also terminates. Discussion of the termination step focuses on what to do after the co-simulation has finished, including how to use the log files to confirm the co-simulation executed properly.
