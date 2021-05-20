# Terminating HELICS

If executing from a C or C++ based program. Ctrl-C should do the right thing. And terminate the local program. If the co-simulation is running across multiple machines then the remaining programs won't terminate properly and will either timeout or if that was disabled potentially deadlock.

## Signal handler facilities

The C shared library has some facilities to enable a signal handler.

```c
/** load a signal handler that handles Ctrl-C and shuts down the library*/
void helicsLoadSignalHandler();

/** clear HELICS based signal Handlers*/
void helicsClearSignalHandler();

```

This function will insert a signal handler that generates a global error on known objects and waits a certain amount time, clears the print buffer, and terminates.

**NOTE** : the signal handlers use unsafe operations, so there is no guarantee they will work. Testing indicates they work in most situations and improve operations where needed but it is not 100 % reliable or code safe.

```c
/** load a custom signal handler to execute prior to the abort signal handler
@details  This function is not 100% reliable it will most likely work but uses some functions and
techniques that are not 100% guaranteed to work in a signal handler
in worst case it could deadlock.  That is somewhat unlikely given usage patterns
but it is possible*/
void helicsLoadSignalHandlerCallback(void (*handler)(int));

```

It is also possible to insert a custom callback into the signal handler chain. Again this is not 100% reliable. But is useful for some language API's that do other things to signals. This allows for inserting a custom operation that do some other cleanup first before calling the normal helics signal handler.

### Signal handlers in C++

facilities for signal handling in C++ were not put in place since it is easy enough for a user to place there own handlers which would like do a better job than any built in ones, so a default one was not put in place at present though may be at a later time.

## Generating an error.

A global error generated anywhere in a federation will terminate the co-simulation.

```c
/**
 * generate a global error through a broker  this will terminate the federation
 *
 * @param broker The broker to set the time barrier for.
 * @param errorCode the error code to associate with the global error
 * @param errorString an error message to associate with the error
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
void helicsBrokerGlobalError(helics_broker broker, int errorCode, const char *errorString, helics_error* err);

void helicsCoreGlobalError(helics_core core, int errorCode, const char* errorString, helics_error* err);

/**
 * Generate a global error from a federate.
 *
 * @details A global error halts the co-simulation completely.
 *
 * @param fed The federate to create an error in.
 * @param error_code The integer code for the error.
 * @param error_string A string describing the error.
 */
HELICS_EXPORT void helicsFederateGlobalError(helics_federate fed, int error_code, const char* error_string);

```

Corresponding functions are available in the C++ API as well. Any global error will cause a termination of the co-simulation.

### Some modifying flags

Setting the `helics_terminate_on_error` flag to true will escalate any local error into global one and terminate the co-simulation.

## Comments

Generally it isn't a wise idea to just terminate the cosimulation without letting everyone else know. If you control everything it probably works fine but as co-simulations get larger more care needs to be taken to prevent zombie processes and hung federates and brokers. Which can cause issues on the next one. This is an evolving area of how best to handle terminating large co-simulations in abnormal conditions and hopefully the best practices will make it easier for users.
