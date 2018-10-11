# Timing #
The [section on federates](./federates.md) addressed the data-exchange responsibility of the co-simulation platform and this will address the timining and synchronization requirements. These two functions work hand-in-hand; for data-exchange between federates to be productive it must be delivered to each federate at the appropriate time. Co-simulation breaks down if federates are simulating differet times (e.g. noon for one, 9am for another) and exchanging data as if they were operating at the same time; the system is no longer coherent.

As discussed in the [section providing the overview of co-simulation operation](./helics_co_sim_sequence.md) the primary mechanism HELICS uses to regulate the time of the individual federates (and thus the federation as a whole), is an iterative process of a federate requesting a simulated time to which it can advance and being granted that time (or another) by the federate's assoicated HELICS broker. For example, a power system simulator may be ready to simulate the next second of operation and once its HELICS broker has determined it is appropriate, it will grant the power system simulator that time.

To be clear, it is the role of each federate to determine which time it should request and it is the job of those integrating the simulator with HELICS to determine how best to estimate that value. For simulators that have no internal mechanisms for changing state (e.g. a power system at steady-state whose loads are time-invariant), a time request for inifinity is made (xxxxxxx - true?). Until an input value changes, these federates have nothing to do and request that they are not granted a time until the co-simulation reaches a conclusion.  If, instead of static load shapes that same simulator was using hourly load profiles, it would make more sense for the federate to make time requests in one-hour intervals.

After making a time request, federates are granted a time by their HELICS broker and the time they are granted will be one of two values: the time they requested or an earlier time. Being granted a time earlier than requested is always accompanied by a new value in one of its subscriptions/endpoints. A change in the federates boundary conditions may require a change in one of the outputs (publications) for that federate and the broker is obliged to wake up the federate so it can process this new information. (There are a few mechanisms by which trivial or nusiance updates for a federate can be ignored and will be discussed later in this section.)

So what does a federate do while its waiting to be granted a time? Generally, nothing. When a federate makes a time request it calls a HELICS function that blocks all other execution in that federate. The federate sits and waits for a return value from that function (the granted time), allowing the rest of the federation to execute. The implication of making a time request is that, given the current state of its boundary conditions, the federate has nothing more to do until the the time it is requesting (or until it receives a new value that changes its boundary conditions from another federate).

Relatedly, not all federates are granted the same simulation time. Thinking of our power system example with hourly loads, it could be that the power system federate makes hourly time requests while the controller, generally, requests inifinity, waiting for a new value to come in for it to act on. The HELICS broker these two are interacting with would then grant the power system federate times of 1pm, 2pm, 3pm, etc while the controller federate sits and waits in the time request blocking function. It would not know that the power system simulator is advancing in simulated time until it is granted a time itself (say, when the voltage at a certain node gets too high and triggers a publication from the power system simulator).

(xxxxxxx - How does a co-simulation end in HELICS?)

The figures below show a few simple examples of how the timing in a co-simulation could proceed.

(xxxxxxx - Co-sim example timing diagrams)


### Timing/Sychronization Options ###
The same JSON configuration file used to set the publications, subscriptions, and endpoints as discussed in the [section on federates](./federates.md) also has a number of parameters that can be set to influence how the federate manages its timing with the co-simulation. As before, there are other parameters that can be set as well and are discussed in (xxxxxxx).

```
{ 
"name":"generic_federate", 
...
"uninterruptible":false,
"period":  1.0,
"offset": 0.0,
...
```
* **uniterruptible [false]** - Normally, a federate will be granted a time earlier than it requested when it recieves a message from another federate; the presence of any message implies there could be an action the federate needs to take and may generate new messages of its own. There are times, though, when it is important that the federate only be granted a time (and begin simulating/executing again) that it has previously requested. For example, there could be some controller that should only operate at fixed intervals even if new data arrives earlier. In these cases, setting the `uninterruptible` flag to `true` will prevent premature time grants.

* **period [xxxxxxxx]** - Many time-based simulators have a minimum time-resolution or a user-configurable step size. The `period` parameter can be used to effectively synchronize the times that are granted with the defined simulation period. xxxxxxx units? Relatedly...

* **offset [0]** - There may be cases where it is preferable to have a simulator receive time grants that are offset slightly in time to one or more other federates. Defining an `offset` value allows this to take place. xxxxxxx - units?

  Setting both `period` and `offset`, will result in the times granted to the federate in question being constrained to `grantTime = n*period + offset`

