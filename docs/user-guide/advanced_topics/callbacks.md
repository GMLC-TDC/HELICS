# Callbacks

Federates have a number of callbacks that can be specified that trigger under different stages of the co-simulation operation or conditions. These callbacks can be used to simplify the management of a federate or enable new capabilities to be integrated with HELICS. This document is a listing and description of the callbacks available on Value, Message, and Combination Federates. Callbacks are user-specified code that is executed inline with other HELICS operations.

In C++, callbacks generally take a std::function object which can be structured as a lambda or direct object. In C and language API's the callback is structured as a function pointer, and pass through a userData object into the callback. HELICS does not manipulate the userData and simply passes it through.

## Specific purpose callbacks

Some callbacks respond to specific events from a federate this includes logging, queries, and errors.

### Logging Callback

The logging callback allows a specific user operation to handle log messages

```C++
void setLoggingCallback(
        const std::function<void(int, std::string_view, std::string_view)>& logFunction);
```

```C
void
    helicsFederateSetLoggingCallback(HelicsFederate fed,
                                     void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                     void* userdata,
                                     HelicsError* err);

```

The main arguments to the callback are a loglevel integer code corresponding to log levels in HELICS, a string identifier, and a log message.

### Query Callback

The query callback allows a federate to respond to custom queries. If a federate receives a query that it does not know the answer to, it executes the query callback if supplied. In C++ the return type is a string, in C and the other language API's the user supplied callback is expected to fill in a query buffer.

```C++
void setQueryCallback(const std::function<std::string(std::string_view)>& queryFunction);
```

```C
void
    helicsFederateSetQueryCallback(HelicsFederate fed,
                                   void (*queryAnswer)(const char* query, int querySize, HelicsQueryBuffer buffer, void* userdata),
                                   void* userdata,
                                   HelicsError* err);

```

### Error Handler Callback

The error handler callback will be executed when an error is encountered and includes arguments for an integer error code and an error message.

```C++
void setErrorHandlerCallback(std::function<void(int, std::string_view)> errorHandlerCallback);
```

```C
void helicsFederateErrorHandlerCallback(HelicsFederate fed,
                                                      void (*errorHandler)(int errorCode, const char* errorString, void* userdata),
                                                      void* userdata,
                                                      HelicsError* err);

```

## LifeCycle Callbacks

Life Cycle callbacks occur at specific stages or transitions in a co-simulation

### Initializing Mode Entry

The InitializingEntry callback is executed when moving into initializing mode. The boolean parameter indicates whether this is iterative. It is set to false the first time this callback is executed (when moving from CREATED mode) and true if returning to this mode from an enterExecutingModeIterative Call.

```C++
void setInitializingEntryCallback(std::function<void(bool)> callback);
```

```C
void helicsFederateInitializingEntryCallback(HelicsFederate fed,
                                                           void (*initializingEntry)(HelicsBool iterating, void* userdata),
                                                           void* userdata,
                                                           HelicsError* err);
```

### Executing Entry

This callback is executed exactly once, when moving from INITIALIZING to EXECUTING mode and has no parameters.

```C++
void setExecutingEntryCallback(std::function<void()> callback);
```

```C
void
    helicsFederateExecutingEntryCallback(HelicsFederate fed, void (*executingEntry)(void* userdata), void* userdata, HelicsError* err);
```

### Time Request Entry

This callback is executed on a time request call, prior to calling the blocking Core API call. The arguments are the current time of the federate, the requested time, and whether an iterative call is being made.

```C++
void setTimeRequestEntryCallback(std::function<void(Time, Time, bool)> callback);
```

```C
void helicsFederateSetTimeRequestEntryCallback(
    HelicsFederate fed,
    void (*requestTime)(HelicsTime currentTime, HelicsTime requestTime, HelicsBool iterating, void* userdata),
    void* userdata,
    HelicsError* err);
```

### Time Update

The time update callback is executed after the Core API returns from a time request, and prior to any value based callbacks executing or having been updated. The arguments are the new time and the boolean argument will be set to true if this is an iterative time.

```C++
void setTimeUpdateCallback(std::function<void(Time, bool)> callback);
```

```C
void helicsFederateSetTimeUpdateCallback(HelicsFederate fed,
                                                       void (*timeUpdate)(HelicsTime newTime, HelicsBool iterating, void* userdata),
                                                       void* userdata,
                                                       HelicsError* err);
```

### Time Request Return

The Time request return callback will execute as the last operation prior to return from a time request. It executes after all value based callbacks, and like the Time Update contains arguments for the new time and an indicator if the time is iterative.

```C++
void setTimeRequestReturnCallback(std::function<void(Time, bool)> callback);
```

```C
void
    helicsFederateSetTimeRequestReturnCallback(HelicsFederate fed,
                                               void (*requestTimeReturn)(HelicsTime newTime, HelicsBool iterating, void* userdata),
                                               void* userdata,
                                               HelicsError* err);
```

### Mode Update

The mode update callback executes whenever the Mode of the federate changes. It will execute prior to timeUpdateCallback when both would be called.
The arguments are the old and new Modes.

```C++
 void setModeUpdateCallback(std::function<void(Modes, Modes)> callback);
```

```C
 void
    helicsFederateSetStateChangeCallback(HelicsFederate fed,
                                         void (*stateChange)(HelicsFederateState newState, HelicsFederateState oldState, void* userdata),
                                         void* userdata,
                                         HelicsError* err);
```

NOTE: notice the different names between the C and C++ API's, this discrepancy is noted and will be corrected in a future release, likely a potential HELICS 4.0 but for now retains the consistency internal to the individual API's.

### Termination

This callback will execute exactly once when the finalize or error state is reached.

```C++
void setCosimulationTerminatedCallback(std::function<void()> callback);
```

```C
void helicsFederateCosimulationTerminationCallback(HelicsFederate fed,
                                                                 void (*cosimTermination)(void* userdata),
                                                                 void* userdata,
                                                                 HelicsError* err);
```

## Value Federate Callbacks

These callbacks will execute when an input is updated and can be general for all inputs or specific to a single one. This callback is not currently available in the C API.

```C++
    void setInputNotificationCallback(std::function<void(Input&, Time)> callback);

    void setInputNotificationCallback(Input& inp, std::function<void(Input&, Time)> callback);
```

## Message Federate Callbacks

These callbacks will execute when an endpoint is updated and can be general for all endpoints or specific to a single one. This callback is not currently available in the C API.

```C++
void setMessageNotificationCallback(const std::function<void(Endpoint&, Time)>& callback);

    void setMessageNotificationCallback(const Endpoint& ept,
                                        const std::function<void(Endpoint&, Time)>& callback);
```
