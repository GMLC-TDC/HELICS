# Timeouts

HELICS has a number of properties to detect things going wrong in the co-simulation. Many of these center around detecting a deadlock condition in federates or even potential bugs in HELICS itself.

Timeouts are the main way this is done. What timeouts make the most sense and how to use them is an ongoing effort and will evolve as the use of HELICS grows and we gain more experience with larger co-simulations.

There are 2 main categories of timeout operations. The first has to do with the connection phase, about how long federates should wait for the broker to be available or network resources to be available before generating an error. The second category deals more with a co-simulation getting stuck and unable to proceed in time either from a single federate hanging or a lost network connection or something of that nature. For small co-simulations it is easy enough to just kill things manually if something goes wrong, but it can be potentially problematic as the cosimulations get larger and more complicated.

## Background

Typical brokers and cores in HELICS operate on a separate thread processing messages in the background from the user code. They also make use of system timers and can generate messages for processing independent of the main loop. The foundation of this is a heartbeat timer on the core and brokers. All options in HELICS that take a time are defaulted to milliseconds if only a number is supplied in the argument field. They also can take a time unit with the number such as `5 s` or `2 min` or `265234 ms`. Using `--tick` sets up the heartbeat which is fundamental to timeouts at the broker and core level. Enabling this requires HELICS be built with ASIO, so in situations where the CMake option `-DHELICS_DISABLE_ASIO=ON` was given all timeouts are non-functional. The tick timer can also be disabled using `--disable_timer`,`--no_tick` or `--debugging`. (NOTE: debugging also changes a few other things). The tick effectively establishes a timing resolution for the timeouts, if the tick is set to 1 second timeouts will have to be exceeded by at least 1 tick for actions to trigger. The default tick is 5 seconds so generally timeouts should be specified in multiples of the tick timer or slightly less than an integer multiple if desired to trigger earlier.

## Co-simulation startup

Starting up a co-simulation often depends on a number of factors. There are multiple pieces in a co-simulation; at least 1 broker, and often several federates. These communicate via network resources. And things can go wrong, so you don't want to have the cosimulation just hang forever waiting on certain pieces, so the most general option is `--timeout`. This is a the timeout for establishing a broker connection, and it is also the default for some other more specific timeouts. The default is 30 seconds. If you are setting up a large co-simulation which may have a lot of contention during the setup phase, it may be necessary to lengthen this, or if you are not starting up the broker before the federates it can be necessary to change to a longer time. Currently 30 seconds is used as a compromise that works in most settings where HELICS is used.

When establishing a network connection sometimes the necessary ports and socket resources are not available in which case the `--network_timeout` applies. This defaults to the `--timeout` but it can be specified independently if desired. The core will use this timeout for retries on network resources before generating an error. The network timeouts are not dependent on the tick timer so will apply even if the timers have been disabled.

## Co-simulation operation

While a cosimulation is executing, bad things can happen; networks can go down, federates can crash. If left on its own this may result in a co-simulation hanging and doing nothing waiting for the failed federate. Detecting and handling this is the primary role of the heartbeat timer. If a broker has not received any communication in a tick, it sends out a ping to its parent or children. IF the parent responds then no action is taken. If no response is received in a certain period of time an error is generated and the process is terminated. Sometime cores are not able to respond to pings in which case the `--slowresponding` flag should be used on that federate/core/broker to prevent co-sim termination from lack of ping responses. The `--debugging` option specified early is really a macro flag which turns on `--slowresponding` and `--disable_timer`. The primary use case is for doing step wise debugging of a federate in a code debugger, otherwise the ticks and other federates could cause issues from the timeouts and pings to the federate being debugged.

At the federate level a `--granttimeout` option is available. This depends on ASIO being compiled in and works similar to the real time mode of federates. At present it is primarily for diagnostics as it cannot generate an error, though this may change at some point in the future. It is disabled by default since some federates could legitimately take a long time to execute so having a timeout on others doesn't make sense. So any grant timeout needs to be handled based on the user knowledge of how the different federates operate. This operation is subject to change but at present the operation is handled in 4 stages.

- 1X timeout print warning message
- 3X timeout request a resend of timing message from blocking dependencies (in case of partial communications failure)
- 6X timeout print timing diagnostic information and send message to parent for additional debugging information
- 10X print additional warning -- this is when additional corrective action may be taken but that is a work in progress

It is expected this process will change as more experience with it is gained.

At the broker level a `--maxcosimduration` option is available. By default it is disabled. If specified on a broker or core the co-simulation will terminate if running for more than the specified total time. This can be useful if running a large number of co-simulations in an automated fashion and the expected time is known. This can prevent a situation where a single co-simulation hangs indefinitely blocking others and doing no useful work.

## Other timeouts

There are two other timeouts that can be specified. The first is a query timeout (`--querytimeout=X`). The default is 15 seconds. This applies to queries which are blocking calls, and if a federate is shutting down or fails as a query is in flight it is possible (not likely) that it could result in a non-response and the query never completing. In that rare situation the query timeout will trigger and result in an error on the query. In very large co-simulations it is possible a complex query could exceed the timeout without there being a problem so it may be necessary to lengthen it.

The `--errortimeout` will specify a certain amount of time to wait after a global error is generated to shutdown the co-simulation network. The default is 10s. This would potentially give time for additional queries and diagnostics in the event of an error. This would trigger if a global error is generated in a federate or core.

## Final notes

The area of timeouts is subject to active development and changes are expected in the future. The operation of timeouts is subject to operating system constraints and timeouts may operate more as a minimum bound on heavily loaded systems with the times exceeding the specified value before action is taken.
