# HELICS Sequence Components

As a co-simulation is, in some sense, a simulation of simulations, there are two levels of configuration required: the configuration of the individual federates as if they were running on their own (identifying models to be used, defining the start and stop time of the simulation, defining how the results of the simulation should be stored, etc...) and the configuration of how each federate will connect to and interact with the other federates in the co-simulation. One of the goals of a co-simulation platform like HELICS is to make the connecting easier and more efficient by providing a standardized method of configuration. To provide a better understanding of why certain types of information are required during configuration and the implications of making these choices, this section presents the components to a HELICS co-simulation in sequence of operation and includes examples of each component from the [Default Co-simulation Example](../examples/fundamental_examples/fundamental_default.md). 

This section provides an overview of the components, each of which are linked to separate sections for more detail. The components discussed are:

```eval_rst
.. toctree::
    :hidden:
    :maxdepth: 1
    
    simulator_integration
    federates
    timing
    helics_cli
    simulate
    logging

```

<!-- This md page has section headers using html tags so that the toctree in the toc pane (left) will direct the user to separate md pages. -->


<a name="co-simulation-design-and-integration">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
Co-simulation Design and Integration
</span>
</strong>
</a>





<a name="federates-and-communication">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
Federates and Communication
</span>
</strong>
</a>

Every federate (instance of a simulator) will require configuration of the way it will communicate (send signals) to other federates in the federation. For simulators that already have HELICS support, the configuration takes the form of a JSON (or TOML) file; bespoke simulators can be configured with the HELICS APIs in the code or via a JSON file. The essential information that HELICS configuration defines is:

   **Federate name** - The unique name this federate will be known as throughout the federation. It is essential this name is unique so that HELICS messages can route properly.

   **Core type** - The core manages interfaces between the federation and the federate; there are several messaging technologies supported by HELICS. 

   **Publications and Inputs** - Publication configuration contains a listing of source handle, data types, and units being sent by the federate; input configuration does the same for values being received by the federate. If supported by the simulator (e.g., [a Python simulator](../examples/fundamental_examples/fundamental_default.md)), these values can be mapped to internal variables of the simulator from the configuration file. 

   **Endpoints** - Endpoints are sending and receiving points for HELICS messages to and from message federates. They are declared and defined for each federate. 

   **Time step size** - This value defines the resolution of the simulator to prevent HELICS from telling the simulator to step to a time of which it has no concept (e.g. trying to simulate the time of 1.5 seconds when the simulator has a resolution of one second). 

In depth explanations for each of these components can be found on the [Federates](./federates.md) page.

<a name="timing">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
Timing
</span>
</strong>
</a>

Once the initialization phase is complete (registration and integration of federates, configuration of signals), the co-simulation proper begins. Every federate will request a simulation time from its core. This time request indicates the point in simulated time at which the federate knows it will have to execute commands so it can simulate some portion of the model or behavior in the system. For example, there may be a federate simulating a building and based on the dynamics of the system, it knows the indoor temperature will not appreciably change over the next five minutes. 

Based on the time requests and grants from all the connected federates, a core will determine the next time it can grant to a federate to guarantee none of the federates will be asked to simulate a point in time that occurs in the past. If the core is doing its job correctly, every federate will receive a time that is the same as or larger than the last time it was granted. HELICS does support a configuration and some other situations that allows a federate to break this rule, but this is a very special situation and would require all the federates to support this jumping back in time, or accept non-causality and some randomness in execution. 




<a name="launch">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
Launch
</span>
</strong>
</a>

Once the communication structure between federates has been configured and the timing architecture established, the co-simulation can be launched. This will create the federates as entities recognized by the broker, set up the communication channels for their messages to be passed, pass some initial messages, and execute some preliminary code as preparation for the beginning of the co-simulation proper. The last step is particularly important if the federates need to reach a self-consistent state as an initial condition of the system.

Execution of the co-simulation is done with `helics-cli`, which condenses the commands to launch each federate into a single executable. Details for how to launch a HELICS co-simulation can be found on the [`helics-cli`](./helics_cli.md) page.



<a name="simulation">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
Simulation
</span>
</strong>
</a>
 
Once a federate has been granted the ability to move forward to a specific time (the granted time), the federate will execute its simulation, calculating its new state, behavior, or control action to advance to that time. Prior to these calculations, it will receive any messages that have been sent to it by other federates and after simulating up to the granted time, may send out messages with new values other federates may need.





<a name="termination-and-debugging">
<strong>
<span style="font-size:larger;color:black;text-decoration:underline;">
Termination and Debugging
</span>
</strong>
</a>

After all possible time grants have been exhausted, the federates signal to their core that they are leaving the federation. Once all the federates have left, the rest of the infrastructure disassembles itself and also terminates. Discussion of the termination step focuses on what to do after the co-simulation has finished, including how to use the log files to confirm the co-simulation executed properly.
