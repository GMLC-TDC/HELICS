# Timing Configuration

```{eval-rst}
.. toctree::
    :hidden:
    :maxdepth: 1

    timing_exercise
    timing_exercise_answers

```

The two fundamental roles of a co-simulation platform are to provide a means of data exchange between members of the co-simulation (federates) and a means of keeping the federation synchronized in simulated time.

In HELICS, time synchronization across the federates is managed by each federate requesting a time (via a HELICS API call). When granted a time, the federate generally does the following:

1. Checks all its inputs and grab any new signals (values or messages) that have been sent to it,
2. Execute its native simulation code and update its state to the current simulated time (_e.g._ solving equations governing the behavior of a physical model, calculating a control action, processing and logging data, etc),
3. Publish new values or send new messages to other federates, and
4. Request the next simulated time to update again.

Sometimes this timing configuration is determined by the construction of the simulator (for example, if it has a fixed simulation time step size) and sometimes the simulator will have nothing to do until it receives a new input (for example, with a controller).

In most federates there will be a line of code that look like this (at least if they are [using the Python API](../../references/api-reference/index.md)):

```python
t = h.helicsFederateRequestTime(fed, time_requested)
```

It is the role of each federate to determine which time it should request and it is the job of those integrating the simulator with HELICS to determine how best to estimate that value. For some simulators, `time_requested` will be the current simulated time (`t`) plus a time step. For other simulators, `time_requested` may be the final time (`HELICS_TIME_MAXTIME`) (see the example on [Combination Federates](../examples/fundamental_examples/fundamental_combo.md) for more details on this), and it will only be granted time (interrupted, configured as `"uninterruptible": false`) when there are relevant updates provided by other federates in the co-simulation. Generally, time requests are blocking calls and our federate will do nothing until the HELICS core has granted a time to it.

When a federate makes a time request it calls a HELICS function that blocks the execution of that thread in HELICS. (If the simulator in question is multi-threaded then other threads can continue to operate; hopefully whatever they're working on is largely independent of the rest of the federation.) The federate sits and waits for a return value from that function (the granted time), allowing the rest of the federation to execute. The implication of making a time request is that, given the current state of its boundary conditions, the federate has no tasks to execute until the time it is requesting, or until it receives from another federate a new value that changes its boundary conditions.

After making a time request, federates are granted a time by their HELICS core and the time they are granted will be one of two values: the time they requested (or the next available valid time as defined by their configuration) or an earlier valid time. Being granted a time earlier than requested is always accompanied by a new value or message in one of its inputs, subscriptions, or endpoints. A change in the federate's boundary conditions may require a change in one of the outputs for that federate and its core is obliged to wake up the federate so it can process this new information.

Based on the time requests and grants from all the connected federates, a core will determine the next time it can grant to a federate to guarantee none of the federates will be asked to simulate a point in time that occurs in the past. Every federate will receive a time that is the same as or larger than the last time it was granted. HELICS does support a configuration and some other situations that allows a federate to break this rule, but this is a very special situation and would require all the federates to support this jumping back in time, or accept non-causality and some randomness in execution.

The [section on federates](./federates.md) addressed the data-exchange responsibility of the co-simulation platform and this will address the timing and synchronization requirements. These two functions work hand-in-hand; for data-exchange between federates to be productive, data must be delivered to each federate at the appropriate time.

<!-- (There are a few mechanisms by which trivial or nuisance updates for a federate can be ignored and will be discussed later in this section.) -->

HELICS co-simulations end under one of two conditions: when all federates have been granted the time of `HELICS_TIME_MAXTIME` or when all federates have notified the broker (via their core) that they are terminating. The termination of the federates triggers a cascade of terminations throughout the federation: once all the federates associated with a core have terminated, the core itself terminates and once all cores associated with a broker have terminated, the broker itself terminates. This concludes the co-simulation and leaves the original models, configuration files, executing simulators, and results files in place for review.

## Timing Configuration Options

Managing the timing of federate co-simulation is one of the most important and often challenging aspects of co-simulation. It is not uncommon for a federation to require that certain federates run at particular times or after certain other federates. HELICS provides a wide variety of timing parameters that can be configured for each federate (see the "Timing" section of the [Configuration Options Reference](../../references/configuration_options_reference.md#timing-options)).

The same JSON configuration file used to set the publications, subscriptions, and endpoints (as discussed in the [section on federates](./federates.md)) also controls how the federate manages its timing within the co-simulation.

Below is an example of how the most common of these are implemented in a federate configuration JSON file:

```json
{
  "name": "generic_federate",
  "period": 1.0,
  "offset": 1.0,
  "time_delta": 10.0,
  "uninterruptible": false,
  "wait_for_current_time_update": true
}
```

`period`, `offset`, and `time_delta` are all related and defined with units of seconds.

- `period`: Defines the resolution of the federate and is often tied to the underlying simulation tool. Period forces time grants to specific intervals.
- `offset`: Requires all time grants to be offset in time from the intervals defined by `period` by the amount indicated.
- `time_delta`: Forces the granted time to a minimum interval from the last granted time.

The granted time will be of value `n*period + offset` and it must be later than the last grant by time `time_delta`.

The other two options are common flags which may be invoked:

- `uninterruptible`: Forces the granted time to be the requested time. Generally HELICS will grant a federate a time when it receives new values on any of its inputs under the assumption that the federate is inherently interested in responding to new information to which it has subscribed. If that is not the case, setting this flag will reduce nuisance grants and move the federate forward in a predictable manner.
- `wait_for_current_time_update`: Force the federate with this flag set to be the last one granted a given time and thereby ensures that all other federates have produced outputs for that time. By being last, the federate in question will have updated outputs from all other federates and have the most comprehensive understanding of the system state at that simulated time.

## Example: Timing in a Small Federation

For the purposes of illustration, let's suppose that a co-simulation federation with the following timing parameters has been assembled:

- **Logger** - This federate is a results logger and simply writes out to files the current values of various publications made by the other federates in the co-simulation. This logging simulator will record values every 1 ms and as such, the JSON config sets `period` to this value and sets the `uninterruptible` flag.
- **Generator** - This is a generator simulator that specializes in comprehensive modeling of the machine dynamics. The Generator will have an endpoint used to receive commands from the Generator Controller and subscriptions to outputs from the Power System that provide inputs necessary to calculate its internal dynamics.

  The models of the generator are valid at a time-step of 0.1 ms and thus the simulator integrator requires that the `period` of the HELICS interface be set to some multiple of 0.1. In this case we'll use 1 ms and to ease integration with the Power System federate, it will also have an `offset` of 0.5 ms.

- **Generator Controller** - This is an event-based simulator, updating the control commands to the Generator federate whenever new inputs are received from the Power System federate (subscriptions to the physical values it calculates). As such, it will always request `HELICS_TIME_MAXTIME`, expecting to be granted times whenever the state of the Power System federate changes. The `timeDelta` will be set to 0.010 ms to replicate the time it takes to calculate and communicate the command signals to the Generator.
- **Power System** - This federate is a classic power system dynamics simulator with a fixed time-step of 1 ms. The integrator of this simulator choose to realize this by setting the `uninterruptible` flag and hard-coding the time requests to advance at 1 ms intervals.

Below is a timing diagram showing how these federates interact during a co-simulation. The filled blocks show when each federate has been woken up and is active.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/helics_timing_example.png)

Items of notes:

- Generator Controller gets granted a time of 1 ms (at the first grant time) even though is requested `HELICS_TIME_MAXTIME` because a message was created by the Power System federate at that time stamp. As Generator Controller depends on nothing else, HELICS was able to grant it the same time as Power System even though it is clearly performing its calculations after Power System has performed its.
- Relatedly, Generator Controller requests a time of `HELICS_TIME_MAXTIME` once it has calculated the new control signals for Generator. Due to the value set by `timeDelta`, the soonest time it can be granted would be 0.01 ms after its most recent granted time (1.01 in the case of the first operational period, 2.01 in the case of the second period.)
- When Logger is granted a time of 1 ms, the values it will record are those previously published by other federates. Specifically, the new values that Power System is calculating are not available for Logger to record.
