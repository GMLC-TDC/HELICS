# Dynamic Federations

In general a dynamic federation is one in which the federates or simulators are not all available at the start of the co-simulation. Instead some components attach to the running co-simulation part-way through the simulation. The longer term goal of HELICS is to fully support dynamic federations but there are many complicated issues to handle timing and data communication that need to be resolved before full general dynamic federations are supported. As of HELICS 3.1 a limited though quite useful form is available through observer federates.

## Levels of Dynamic Federations

Dynamic Federations come in various flavors of complexity. The first is simply allowing additional subscriptions to existing federates. The next is allowing additional "observer" federates to join a running co-simulation. The third is allowing new publications, endpoints, or filters on existing federates. And the fourth is a full dynamic federation.

The first of these has been unofficially available for some time and will be supported in HELICS 3.1. Dynamic observer federates will also be allowed in HELICS 3.1, and will be detailed further in a subsequent section. The last two levels are currently a work in progress and are currently planned to be supported in HELICS 3.2 (Early 2022).

### Dynamic Subscriptions

In the normal case, the `registerInput`, `registerSubscription`, or `registerTargetedEndpoint` are done in the creation phase of co-simulation so all the checks and connections can be made and everything set up before `timeZero` (start of co-simulation). If these calls are made after the `enterInitializingMode` call, then the time guarantees are different. Ordinarily data published in the initialization phase would be available to all subscriptions of a publication, but with dynamic subscriptions the data is not going to be sent to the new subscription until new publications after a time request or `enterExecutingMode` call has been made. (A `bufferData` flag is in the works to allow data to be sent for dynamic subscriptions at the time of connection if set on the publisher).

Apart from time considerations there are no additional requirements. The checks on the subscription are done immediately as opposed to waiting for the `enterInitializingMode` call.

### Dynamic Observer Federates

A federate may declare itself to be an observer in the FederateInfo structure when a federate is declared. This can be done via the command line (`--observer`) or through a flag.

In the C++ API

```c++
FederateInfo fi;
fi.observer=true;
```

and in the C or other language API's

```c
helicsFederateInfoSetFlagOption(fi, HELICS_FLAG_OBSERVER,HELICS_TRUE,&err);
```

The observer flag triggers some flags in the created core and federate to notify the broker that it can be dynamically added. Otherwise it will get an error notification that the federate is not accepting new federates. If a core is created before the new federate it must also be created with the observer flag enabled. Otherwise, it will not be allowed to join the federation.

Once joined, subscriptions can be added with the same timing rules as described in the previous section. One key thing to be aware of is that for dynamic federates the time returned after enterExecutingMode is not necessarily 0, but will depend on the time of federates containing the publications or endpoints that are being linked. If there are no data pathways zero will still be returned.

The utility of this capability is primarily for debugging/observation purposes. It is possible to join, get the latest data and make some queries about the current co-simulation status for monitoring purposes and disconnect. At present a new name is required each time an observer connects. This may be relaxed in the future.

### Dynamic Publications

Dynamic publications and endpoints will be allowed in the near future. These would include just creating a publication or endpoint on an already executing federate. It would then be available for others to connect to and receive data. It would typically take at least two timesteps of the publisher to actually receive any data from the new publication. The first to ensure registration is completed and the second to publish the data. It would be possible to send the data immediately after the register call but it would be impossible (unless on the same core) for any other federate to connect until after the second request time call.

### Full Dynamic Federations

Allowing full dynamic federations will require activating a flag on the root broker to explicitly allow dynamic federations. It is not intended to be a normal use case, and will not be enabled by default. This functionality is not operational as of HELICS 3.1 and significantly more testing is needed before the feature becomes generally available and more details will be added at that time.
