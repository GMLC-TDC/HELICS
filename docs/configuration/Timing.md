# Federate Timing

Time contrlol in a federation is handled via timeController objects in each 
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
The minimum time delta of federate determines how close two granted times may be to eachother
The default value is set to the system epsilon which is the minimum time resolution of the Time class
used in HELICS.  This can be used to achieve similar effects as the period, but it has a different meaning.
If the period is set to be smaller than the minTimeDelta, then when granted the federate will skip ahead a couple timesteps.

With these parameters many different patterns are possible.  

### Input Delay
The input Delay can be thought of as the propagation delay for signals going into a federate
Basically all values and signals are only acknowledged in the timing calculations after the prescribed delay

### Output Delay
The output delay is the match to the InputDelay.   
