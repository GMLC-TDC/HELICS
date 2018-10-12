# Federate info

The FederateInfo structure contains information that can be passed to a federation upon construction. Some information can be updated continuously other can be only be changed before initializationMOde is entered.

**separator [char]**

the separator character between federateName and endpoint or publications that are not declared global.  the default is '/'

**coreName  [string]**

The name of the core to connect with,  can be left blank to either find an available core or generate one automatically.

**CoreInitString [string]**

Command line arguments that are passed to the core when starting it.  Some examples are
  - "2" to specify 2 federates will connect
  - "1 --broker=192.168.2.3:23444"  to specify a single federate and to connect to a broker at ipaddress 192.168.2.3 port 23444

**coreType[enum]**

Specify which type of core to use.  see [core types](CoreTypes.md) for more details
  - DEFAULT = 0,  //!< pick a core type depending on compile configuration usually either ZMQ if available or UDP
  - ZMQ = 1,  //!< use the Zero MQ networking protocol
  - MPI = 2,  //!< use MPI for operation on a parallel cluster
  - TEST = 3,  //!< use the Test core if all federates are in the same process
  - INTERPROCESS = 4,  //!< interprocess uses memory mapped files to transfer data (for use when all federates are
                      //!< on the same machine
  - IPC = 5,  //!< same as INTERPROCESS
  - TCP = 6,  //!< use a generic TCP protocol message stream to send messages
  - UDP = 7,  //!< use UDP packets to send the data
  the enumeration names in the C interface may be different.

## Timing control variables

see [timing](./Timing.md) for more details.

**timeDelta[time]**

the minimum time advance allowed by the federate
default timeEpsilon

**outputDelay[time]**

The amount of time values and messages take to propagate to be
available to external federates.
default= 0

**inputDelay[time]**

the time it takes values and messages to propagate to be accessible to the Federate
default=0

**period[time]**

a period value,  all granted times must be on this period n*Period+offset
default=0

**offset[time]**

offset to the time period
default=0

**rt_lag[time]**

real time tolerance - the maximum time grants can lag real time before HELICS automatically acts
default=0.2 given this operates on a computer clock, time <0.005 are not going be very accurate or followed that closely unless the OS is specifically setup for that sort of timing level

**rt_lead[time]**

real time tolerance - the maximum time grants can lead real time before HELICS forces an additional delay
default 0.1

## Timing flags

 - `observer` = false
 flag indicating that the federate is an observer
 - `uninterruptible` =false
flag indicating that the federate should never return a time other than requested
 - `source_only` = false;
flag indicating that the federate does not receive or do anything with received information.
 - `only_transmit_on_change` =false
flag indicating that values should only updated if the number has actually changes
 - `only_update_on_change` = false
flag indicating values should be discarded if they are not changed from previous values
 - `wait_for_current_time_updates` = false
flag indicating that the federate should only grant when no more messages can be received at the current time
 - realtime = false
flag indicating that the federate is required to operate in real time.  the federate must have a non-zero period

## Other Controls

**maxIterations[int16]**

the maximum number of iterations allowed for the federate
default=50

**logLevel[int32]**

the logging level above which not to log to file default 1(WARNING)
