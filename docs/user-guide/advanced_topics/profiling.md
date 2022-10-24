# Profiling

As of versions 2.8 or 3.0.1 HELICS includes a basic profiling capability. This is simply the capability to generate timestamps when entering or exiting HELICS blocking call loops where a federate may be waiting on other federates.

## Output

The profiling output can be either in the other log files or a separate file, and can be enabled at the federate, core, or broker levels.
There are 3 messages which may be observed:

```text
<PROFILING>test1[131072](created)MARKER<138286445040200|1627493672761320800>[t=-9223372036.854776]</PROFILING>
<PROFILING>test1[131072](initializing)HELICS CODE ENTRY<138286445185500>[t=-1000000]</PROFILING>
<PROFILING>test1[131072](executing)HELICS CODE EXIT<138286445241300>[t=0]</PROFILING>
<PROFILING>test1[131072](executing)HELICS CODE ENTRY<138286445272500>[t=0]</PROFILING>
```

The messages all start and end with <PROFILING> and </PROFILING> to make an xml-like tag.
The message format is `FederateName[FederateID](federateState)MESSAGE<wall-clock time>[simulation time]`

The federate state is one of `created`, `initializing`, `executing`, `terminating`, `terminated`, or `error`.

The three possible `MESSAGE` values are:

- `MARKER` : A time stamp matching the local system up time value with a global time timestamp.
- `HELICS CODE ENTRY` : Indicator that the executing code is entering a HELICS controlled loop
- `HELICS CODE EXIT` : Indicator that the executing code is returning control back to the federate.

For `HELICS CODE ENTRY` and `HELICS CODE EXIT` messages the time is a steady clock time, usually the time the system on which the federate is running has been up. The `MARKER` messages have two timestamps for global coordination, `<steady clock time|system time>`. The system time is the wall clock time as available by the system, which is usually with reference to Jan 1, 1970 and in GMT.

The timestamp values are an integer count of nanoseconds. For all 3 message types they refer to the system uptime which is monotonically non-decreasing and steady. This value will differ from each computer on which federates are running, though. To calibrate for this there is a marker that gets triggered when the profiling is activated, indicating the local uptime that is synchronous across compute nodes. This matches a system uptime, with the global system time. The ability to match these across multiple machines will depend on the latency associated with time synchronization across the utilized compute nodes. No effort is made in HELICS to remove this latency or even measure it; that is, though the marker time is measured in nanoseconds it could easily differ by microseconds or even milliseconds depending on the networking conditions between the compute nodes.

## Enabling profiling

Profiling can be enabled at any level of the hierarchy in HELICS and when enabled it will automatically enable profiling on all the children of that object. For example, if profiling is enabled on a broker, all associated cores will enable profiling and all federates associated with those cores will also have profiling enabled. This propagation will also apply to any child brokers and their associated cores and federates.

### Broker profiling

Profiling is enabled via the command prompt by passing the `--profiler` option when calling `helics_broker`.

- `--profiler=save_profile2.txt` will clear save_profile2.txt and save new profiling data to a text file `save_profile2.txt`
- `--profiler_append=save_profile2.txt` will append profiling data to the text file `save_profile2.txt`
- `--profiler=log` will capture the profile text output to the normal log file or callback
- `--profiler` is the same as `--profiler=log`

Enabling this flag will pass in the appropriate flags to all children brokers and cores.

### Core profiling

Profiling is enabled via the `coreinitstring` by adding a `--profiler` option.

- `--profiler=save_profile2.txt` will clear save_profile2.txt and save new profiling data to a text file `save_profile2.txt`
- `--profiler_append=save_profile2.txt` will append profiling data to the text file `save_profile2.txt`
- `--profiler=log` will capture the profile text output to the normal log file or callback
- `--profiler` is the same as `--profiler=log`

Enabling this flag will pass in the appropriate flags to all children federates.

### Federate profiling

Profiling on a federate will recognize the same flags as a core, and pass them as appropriate to the core. However a federate also supports passing the flags and a few additional ones into the federate itself.

```c
helicsFederateSetFlagOption(fed,HELICS_FLAG_PROFILING, HELICS_TRUE, &err);
```

can directly enable the profiling. If nothing else is set this will end up generating a message in the log of the root broker.

```c
helicsFederateSetFlagOption(fed,HELICS_FLAG_PROFILING_MARKER, HELICS_TRUE, &err);
```

can generate an additional marker message if logging is enabled.

```c
helicsFederateSetFlagOption(fed,HELICS_FLAG_LOCAL_PROFILING_CAPTURE, HELICS_TRUE, &err);
```

captures the profiling messages for the federate in the federate log instead of forwarding them to the core or broker.

Some can be set through the flags option for federate configuration.
`--flags=profiling,local_profiling_capture` can be set through command line or configuration files. If enabling the `local_profiling_capture`, `profiling` must also be enabled; that is, just setting `local_profiling_capture` does not enable profiling. The profiling marker doesn't make sense anywhere but through the program call.

## Notes

This capability is preliminary and subject to change based on initial feedback. In HELICS 3 there will probably be some additional command infrastructure to handle profiling as well added in the future.

If timing is done in the federate itself as well there will be a time gap; there is some processing code between the profiling message, and when the actual function call returns, but it is not blocking and should be fairly short, though dependent on how much data is actually transferred in the federate. Profiling does not work with callback federates.
