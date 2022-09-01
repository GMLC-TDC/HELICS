# Callback Federates

Starting in HELICS v3.3 Callback federates were added as way to entirely inline the operation of federate allowing significant increases in the number of federates that could partake in a co-simulation for a given system size. Callback federates are a specific type of federate that adds some additional functionality to implement a federate. This includes a couple new callbacks and properties not available in a regular federate.

## Additional Callbacks

The Callback Federate adds a few additional callbacks necessary for operation

### Initialize

The initialize callbacks' purpose is to allow a callback federate to attempt to enter executingMode iteratively if desired. The return value is an IterationRequest enumeration. If the callback is not specified for a callback federate NO_ITERATIONS is assumed.

```C++
void setInitializeCallback(std::function<IterationRequest()> initializeCallback);


void helicsCallbackFederateInitializeCallback(HelicsFederate fed,
                                                            HelicsIterationRequest (*initialize)(void* userdata),
                                                            void* userdata,
                                                            HelicsError* err);
```

### Time requests

The purpose of the following callbacks is to allow a user to specify what the next time request should be. This can be done as a simple time or as an iteration_time if iterations are needed. If the iteration time callback method is specified it overrides the simpler callback. The callback must be cleared if desired to revert either through a NULL object or the `clearNextTimeCallbacks` operation.

```C++
void setNextTimeCallback(std::function<Time(Time)> nextTimeCallback)

void helicsCallbackFederateNextTimeCallback(HelicsFederate fed,
                                                          HelicsTime (*timeUpdate)(HelicsTime time, void* userdata),
                                                          void* userdata,
                                                          HelicsError* err);

```

```C++
void setNextTimeIterativeCallback(
        std::function<std::pair<Time, IterationRequest>(iteration_time)> nextTimeCallback);

void helicsCallbackFederateNextTimeIterativeCallback(
    HelicsFederate fed,
    HelicsTime (*timeUpdate)(HelicsTime time, HelicsIterationResult, HelicsIterationRequest* iteration, void* userdata),
    void* userdata,
    HelicsError* err);

```

There is also a method to clear the time callbacks.

```C++
void clearNextTimeCallback();
```

All callback are optional.

## Properties

Callback federates define an additional property `HELICS_PROPETY_TIME_MAXTIME` this cause the callback federate to finalize once a time returned is greater or equal to the specified maxtime.

### Other key properties

Specifying a period `HELICS_PROPERTY_TIME_MAXTIME` will allow the callback federate to automatically compute the next time with no additional callback for the time computation.

Specifying that the federate is event driven will send the next request as maxTime which will grant only when there is some new data to process. This can specified with `HELICS_FLAG_EVENT_TRIGGERED`.

## Execution

Callback federates are combination Federates so any operation that works on combination federates works on Callback Federates. Callback federates behave identically to regular federates until a call to enterInitializingMode(). In regular federates this call is blocking, in Callback federates this call is an asynchronous call and will transfer control of the federate to the HELICS core. All other blocking calls in HELICS will produce an error. All callbacks defined in a regular federate are available for use and are expected to be used. The additional callbacks defined here are to control the next time step and iterations which are directly user controlled in normal federates.

## Expected Use cases

Callback federates allow the potential for a large number of small federates to execute in a core, up to 131,072 in each core. Trying this with normal federates rapidly exceeds the thread capacity of a typical system, so in the cases where the federate can be defined in terms of callbacks and has only a few simple connections and not much computation the callback federate will be much more efficient.
