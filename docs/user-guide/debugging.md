# Debugging

Debugging a co-simulation can be challenging. There are often many components running a on multiple machines and traditional debugging doesn't work that well for coordinating since the processes are often independent.
Enhancing the tools for debugging a HELICS based co-simulation is one area of focus for the coming year and we expect significant changes and improvements as we gain experience in this area. There are now a few basic capabilities in HELICS that support and help with debugging. Those capabilities are outlined in this document and will continue to be expanded.

## Logging

HELICS provides built-in support for various types of logging from federates, brokers, and more. See [Logging](./logging.md) for more details on how to enable and use.

## Players and Recorders

Often an early step in debugging is to isolate the problem. HELICS provides the [**Recorder**](../apps/Recorder.md) and [**Player**](../apps/Player.md) apps to help with doing so for federates and to enable modular development and testing of federations even before all of the federates are working:

- A recorder is just that: a tool that readily records some or all of the traffic that is sent via HELICS (both value-based publication/subscription and message end points). Looking through this output can confirm that the data and timing in HELICS behaves as expected.
- The basic idea for a player is that rather than running all of the other federates, you can test out a single federate or subset by reading the data that other federates would send from a file instead.

Players and recorders can also complement each other, since the files created by a recorder can be played back directly by the players, making it possible to take the partial results from a part of a federation as a recording and then play back--without possible delays or other challenges from the other federates--to troubleshoot the rest of the federation.

## Queries

Queries are regarded as a key component of debugging. They are asynchronous from the main simulation timing and can be used from anywhere in the co-simulation. There are a number of queries that can be used to get the publications, inputs, endpoints, and filters in a co-simulation and get structures with the connections between them. See [Queries](./queries.md) for more details on the specific queries and how to execute them.

## HELICS-CLI

The [HELICS CLI tool](./helics_cli.md) is a tool that can help set up a co-simulation and will eventually have significant debugging capabilities which make use of the underlying capabilities documented here. It is recommended that this tool be used to help with debugging.

## Global Time Barrier

The capability of halting execution at a particular simulation time is a fundamental underlying capability for debugging a co-simulation. The global time barrier capability in HELICS will prevent any simulation that has not reached the given time from granting any time greater or equal to the barrier time. If the barrier is issued and a federate is already passed the given time, it will be blocked at its current time. Once a barrier is issued it will not halt an executing co-simulation until a `requestTime` operation is called by the federate. There are two mechanisms for creating a barrier, some API calls on the broker, and through a REST API in the webserver.

### API calls

The Broker API has two functions related to the time barrier

```c++
setTimeBarrier(Time time);
clearTimeBarrier();
```

For the C API

```C
helicsBrokerSetTimeBarrier(helics_broker brk, helics_time time, helics_error *err);
helicsBrokerClearTimeBarrier(helics_broker brk);
```

For Python and the other language API's

```python
helicsBrokerSetTimeBarrier(helics_broker brk, helics_time time)
helicsBrokerClearTimeBarrier(helics_broker brk)
```

The first function creates or updates a time barrier.
The second clears it. There are no restrictions on what can be done. Simulations start at time 0 so giving a negative time to the `setTimeBarrier` operation will effectively clear the barrier.

### Webbserver interface

Barriers can also be created and updated through the [Webserver](./webserver.md). This includes the websocket server and the HTTP REST API components of the webserver.

Barriers can be created or updated with an HTTP post command
`<webserver>\<broker_name>\barrier` with `time=<value>` in the body. The time can be specified as decimal seconds of simulation or as a value+unit such as "15 ms" or "34 min"

They can be deleted through an HTTP delete command to `<webserver>\<broker_name>\clear_barrier`

Json instructions are also accepted to a websocket interface. Some examples follow:

```json
{
  "command": "barrier",
  "broker": "broker1",
  "time": 15.5
}
```

```json
{
  "command": "barrier",
  "broker": "broker1",
  "time": "275 ms"
}
```

```json
{
  "command": "barrier_clear",
  "broker": "broker1"
}
```

It is expected that some additional mechanics for handling time barriers and manipulating them at finer granualarity will be available in the future and particularly in HELICS 3.0
