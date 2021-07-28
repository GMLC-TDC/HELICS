# Profiling

As of HELICS 2.8 or HELICS 3.0.1 HELICS includes a basic profiling capability.  This is simply the capability to generate timestamps when entering or exiting HELICS blocking call loops where a federate may be waiting on other federates.  

## Output
The profiling output can be either in the other log files or a separate file, and can be enabled at the federate, core, or broker levels.  
There are 3 messages which may be observed

```txt
<PROFILING>test1[131072](created)MARKER<138286445040200|1627493672761320800></PROFILING>
<PROFILING>test1[131072](initializing)HELICS CODE EXIT<138286445153700></PROFILING>
<PROFILING>test1[131072](initializing)HELICS CODE ENTRY<138286445185500></PROFILING>
<PROFILING>test1[131072](executing)HELICS CODE EXIT<138286445241300></PROFILING>
<PROFILING>test1[131072](executing)HELICS CODE ENTRY<138286445272500></PROFILING>
```

The messages all start and end with <PROFILING> and </PROFILING> to make an xml like tag.  
the message format is `FederateName[FederateID](federateState)MESSAGE<timestamp>`

the federate state is one of created, initializing, executing, terminating, terminated, error

The three possible `MESSAGE` values are: , , and
- `MARKER` : A time stamp matching the local system up time value with a global time timestamp.
- `HELICS CODE ENTRY` : Indicator that the executing code is entering a HELICS controlled loop
- `HELICS CODE EXIT` : Indicator that the executing code is returning control back to the federate.

The timestamp values are an integer count of nanoseconds.  for all 3 message types they are a system uptime which is monotonically non-decreasing and steady.  But this is going to be different on every system.  Therefore there is a marker that gets triggered when the profiling is activated.  This matches a system up time, with the global system time.  The ability to match these across multiple machines will depend on the time synchronization of those machines with each other.  No effort is made in HELICS to do that or even measure it.  

## Enabling profiling

profiling can be enabled at any level of the hierarchy in HELICS.  And this will automatically enable profiling on all the children of that object.  

### Broker profiling

Profiling is enabled via the command prompt by passing the `--profiler` option.  

- `--profiler=save_profile2.txt`  will save profiling data to a text file `save_profile2.txt`
- `--profiler=log` will capture the profile text output to the normal log file or callback
- `--profiler` is the same as `--profiler=log`

this will pass in the appropriate flags to all children brokers and cores.  

### Core profiling

Profiling is enabled via the command prompt by passing the `--profiler` option.  

- `--profiler=save_profile2.txt`  will save profiling data to a text file `save_profile2.txt`
- `--profiler=log` will capture the profile text output to the normal log file or callback
- `--profiler` is the same as `--profiler=log`

this will pass in the appropriate flags to all children federates.  

### Federate profiling

Profiling on a federate will recognize the same flags as a core, and pass them as appropriate to the core.  However a federate also supports passing the flags and a few additional ones into the federate itself.  

```
helicsFederateSetFlagOption(fed,HELICS_FLAG_PROFILING, HELICS_TRUE, &err);
```

can directly enable the profiling.  If nothing else is set this will end up generating a message in the log of the root broker.

```
helicsFederateSetFlagOption(fed,HELICS_FLAG_PROFILING_MARKER, HELICS_TRUE, &err);
```

can generate an additional marker message is logging is enabled.

```
helicsFederateSetFlagOption(fed,HELICS_FLAG_LOCAL_PROFILING_CAPTURE, HELICS_TRUE, &err);
```

will capture the profiling messages for the federate in the federate log instead of forwarding them to the core or broker.

Some can be set through the flags option for federate configuration.  
`--flags=profiling,local_profiling_capture`  can be set through command line or configuration files.  
the profiling marker doesn't make sense anywhere but through the program call.  


## Notes
This capability is preliminary and subject to change based on initial feedback. In HELICS 3 there will probably be some additional command infrastructure to handle profiling as well added in the future.   

If timing is done in the federate itself as well there will be a time gap, there is some processing code between the profiling message, and when the actual function call returns, but it is not blocking and should be fairly short though dependent on how much data is actually transferred in the federate.  
