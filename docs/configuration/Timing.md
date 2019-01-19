# Federate Timing

Time control in a federation is handled via timeController objects in each
Federate and Core.  This allows Federation timing to be handled in a distributed
fashion and each federate can tune the timing in a way that is appropriate for the
Federate.

The parameters associated with the time control are in FederateInfo
They incude inputDelay, outputDelay, period, minTimeDelta, and offset.
These parameters along with the timeRequest functions determine how time advances
in a federate.

### Period and Offset
The period and offset of a Federate determine the allowable times which a federate
may grant.  All granted times for a federate will be in accordance with

T=n*Period+offset

With the exception that all federates are granted time=0 when entering execution mode.
n can be 0 so if the offset is greater than 0 then the first granted time will T=offset.
The defaut values for both period and offset are 0.  Offset can be set to a value bigger than the
period if a federate wishes to skip ahead and ignore transients or other updates going on in the first
part of a co-simulation.

### minTimeDelta
The minimum time delta of federate determines how close two granted times may be to each other
The default value is set to the system epsilon which is the minimum time resolution of the Time class
used in HELICS.  This can be used to achieve similar effects as the period, but it has a different meaning.
If the period is set to be smaller than the minTimeDelta, then when granted the federate will skip ahead a couple timesteps.

With these parameters many different patterns are possible.  

### Input Delay
The input Delay can be thought of as the propagation delay for signals going into a federate
Basically all values and signals are only acknowledged in the timing calculations after the prescribed delay

### Output Delay
The output delay is symmetrical to the input delay.  Except it applies to all outgoing messages.  Basically once a time is granted the federate cannot effect
other federates until T+outputDelay

#### rt_lag
real time tolerance - the maximum time grants can lag real time before HELICS automatically acts
default=0.2 given this operates on a computer clock, time <0.005 are not going be very accurate or followed that closely unless the OS is specifically setup for that sort of timing level
#### rt_lead
real time tolerance - the maximum time grants can lead real time before HELICS forces an additional delay

## Timing Flags

### uninterruptible
if set to true the federate can only return time expressly requested(or the next valid time after the requested time)

### source_only
indicator that the federate is only used for signal generation and doesn't depend on any other federate for timing.
Having subscriptions or receiving messages is still possible but the timing of them non-deterministic.

### observer
If the observer flag is set to true, the federate is intended to be receive only and will not impact timing of any other federate
sending messages from an observer federate is undefined.  

### rollback (not used)
Should be set to true for federates that support rollback

### only_update_on_change
If set to true a federate will only trigger a value update if the value has actually changed on a granted time.
Change is defined as binary equivalence,  Subscription objects can be used for numerical limits and other change detection.

### only_transmit_on_change
if set to true a federate will only transmit publishes if the value has changed.  Change is defined as binary equivalence.
If numerical deltas and ranges are desired use Publication objects for finer grained control.  This flag applies federate wide.

### wait_for_current_time_update
if set to true a federate will wait on the requested time until all other federates have completed at least 1 iteration of the current time or have moved past it.  If it is known that 1 federate depends on others in a non-cyclic fashion, this can be used to optimize the order of execution without iterating.  

### realtime
if set to true the federate uses rt_lag and rt_lead to match the time grants of a federate to the computer wall clock.  If the federate is running faster than real time this will insert additional delays.  If the federate is running slower than real time this will cause a force grant, which can lead to non-deterministic behavior.  rt_lag can be set to maxVal to disable force grant
