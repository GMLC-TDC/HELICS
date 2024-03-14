# Dynamic Federations

In general, a dynamic federation is one in which one or more federates joins the co-simulation after the co-simulation begins. Instead, some federates join the running co-simulation part-way through its execution. For example, a co-simulation with EV federates being charged by a power system federate may have EV federates joining the co-simulation to charge and leave it when charging is done. Alternatively, real-time or hardware-in-the-loop co-simulations may have federates (components) that are only needed for part of the co-simulation and join late. Dynamic federations provide greater flexibility in constructing and running co-simulations.

## Levels of Dynamic Federations

Dynamic federations can be thought of as being composed of features in increasing levels of complexity:

1. Allowing the additional of subscriptions to an existing federate publication by existing members of the federation.
2. Allowing federates that only receive information ("observers") to join the federation after execution has begun. As a part of joining the co-simulation the observer would need the functionality in level one to successfully subscribe to the necessary publications of other federates.
3. Allowing the creation of new publications, endpoints, or filters by existing federates which other members of the federation could then connect to as targets.
4. Allowing federates to join the co-simulation after execution has begun and create arbitrary interfaces (publications, subscriptions, endpoints, etc). This relies on all previous levels of complexity being implemented.
5. Allowing federate to disconnect and reconnect throughout the simulation.

HELICS v3.1 supported levels 1 and 2. HELICS v3.4 supports full dynamic federations (level 4). Level 5 is supported as of version v3.5.1.

## Dynamic Subscriptions

In the normal case, the `helicsFederateRegisterInput()`, `helicsFederateRegisterSubscription()`, or `helicsFederateregisterTargetedEndpoint()` methods are called in the creation phase of co-simulation to allow for the creation of the data exchanges between federates prior to the start of co-simulation. If these calls are made after the `helicsFederateEnterInitializingMode()` call, the topology of the data exchanges between federates is altered and with it the timing dependencies. In non-dynamic federations, data published in the initialization phase is available to all subscribers of that publication as soon as as any other federate enters executing mode or makes a time request. In a dynamic federation, subscription data by default is not sent to the new (dynamically added) subscriber (or subscription) until a new publication is made after a time request or `helicsFederateEnterExecutingMode()` call has been made by the subscriber. For example, let's say Federate A publishes a value at simulation time zero and never after that point. If Federate B joins the co-simulation late and enters executing mode at simulation time five, it will not see the value published by Federate A.

In many use cases, this lack of visibility to previously published values is a problem. For example, if the published value is a voltage, it would make sense that this value would be available to all federates even if they join late; the voltage always exists and thus should conceptually always be accessible for use by the model inside the federate. To provide persistence in this data for dynamic co-simulations, each publication can set a `bufferData` flag that retains the last value published so that any late-joining federates can access it, and this value will be sent immediately when the connection is made. This buffer is disabled by default as it does slightly increase the memory footprint of each federates using it. It is anticipated that for dynamic federations, many use cases will want to employ this flag to preserve the "always available" concept of published values. The flag can be set at the federate level so all created publications have the flag set automatically.

## Dynamic Observer Federates

A federate may declare itself to be an observer in the FederateInfo structure when a federate is declared. This can be done via the command line (`--observer`) or through a HELICS federate flag as shown below. This declares that the federate will only be receiving data, not sending any so there is no time dependency of any other federate to this one.

C++:

```c++
FederateInfo fi;
fi.observer=true;
```

C:

```c
helicsFederateInfoSetFlagOption(fi, HELICS_FLAG_OBSERVER,HELICS_TRUE,&err);
```

Python:

```python
import helics as h

fi = h.helicsCreateFederateInfo()
h.helicsFederateInfoSetFlagOption(fi, h.HELICS_FLAG_OBSERVER, True)
```

The observer flag triggers functionality in the corresponding HELICS core and federate to notify the broker that it can be dynamically added. If this flag is not set an error message will be produced indicating that the federation is not accepting new federates. If a HELICS core is created before the new federate it must also be created with the observer flag enabled.

Once joined, subscriptions can be added with the timing limitations as described in the previous section. Be aware that dynamic federates, after calling `helicsFederateEnterExecutingMode()`, the time returned is not necessarily zero, but will depend on the time of federates containing the publications or endpoints that are being linked. The late-joining observer federate can get current simulation time by calling `helicFederateGetCurrentTime()`, as necessary.

Observer federates are useful for co-simulation debugging and monitoring purposes. Using an observer, it is possible to join the federation, get the latest data and make some queries about the current state of the co-simulation and then disconnect. At present, a new name is required each time an observer connects.

## Dynamic Publications

Dynamic publications and endpoints are implemented as of HELICS v3.4. For any late-joining federate, the process by which the federate joins can introduce delays in subscribers seeing the values that are published. Just as in a static co-simulation, after calling `helicsFederateEnterExecutingMode()` a federate can publish values and these values will be available to any subscribing federates the next time they make a time request. This time request is also the point at which a federate could add a subscription to the late-joining federate's publication; that is, the simulation time at which the publications of the late-joining federate become visible to the rest of the federation. No values published by the late-joining federate will be visible to the federate adding this new subscription until a subsequent time request is granted.

## Full Dynamic Federations

Given the above limitations, as of HELICS v3.4 fully dynamic federations are supported. By setting the `--dynamic` flag on the root broker of a federation and any intermediate brokers or cores to which dynamic federates may be added, federates may join the federation late. (HELICS have always been able to leave a federation early.) And as with observer federates, after calling `helicsFederateEnterExecutingMode()`

## Reentrant federates

As of v3.5.1 reentrant federates are allowed. They must be specified with the reentrant flag (`--reentrant`) and be used in a dynamic federation. This allows a federate to disconnect, then rejoin with the same name at a later time in the co-simulation. The second and later joins are like a dynamic federate in terms of timing. Interfaces connecting to a reentrant federate may use the HELICS_OPTION_FLAG_RECONNECTABLE to allow for dynamic/automatic reconnections when an interface with the same name is created from a reentrant federate. This may be done on the same core or a new/different core. The new (reentrant) federate does not inherit any properties (other than the name) from the previous federate with the same name.

## Example

An example of dynamic federation operation is under development though HELICS makes it very easy to support a dynamic federation. Simply add `--dynamic` to the broker initialization string for the root broker (if you are employing a [broker hierarchy](./broker_hierarchies.md)). For example, in a federation with four federates (one of which will be joining late), the call to start the broker is

```shell
$ helics_broker -f3 --dynamic
```

In this example, three federates must be in the federation in order to enter initialization and then executing mode and the broker is ready if more join later.
