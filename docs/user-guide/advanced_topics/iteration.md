# Iteration

In simulation in general iteration at a time step is needed in the case of [algebraic loops](https://www.mathworks.com/help/simulink/ug/algebraic-loops.html), that is, when two state variables are co-dependent on each other.
In these situation, different methods (generally Fixed-Point Methods, like Newton's method), can be used to arrive at the desired solution.

Since in co-simulation, the states are not all visible to any one solver/federate, the situation is a bit trickier.
This is part of the reason, co-simulation is sometimes viewed as the tool-of-last-resort.

Iterations at a time step are intended to resolve this issue by allowing Federates to exchange data back and forth until convergence to the fixed point is reached.

```{note}
Since neither federate has a full view of the problem, convergence issues cannot be dealt with quite in the same way as general numerical solvers. In fact, convergence cannot be guaranteed, because no federate really knows the logic/trajectory of the other federates it is iterating against.
```

## Calling Functions

There are two possible states where iterations are possible in HELICS:

| State            | Iteration call                                |
| ---------------- | --------------------------------------------- |
| `INITIALIZATION` | `helicsFederateEnterExecutingModeIterative()` |
| `EXECUTION`      | `helicsFederateRequestTimeIterative()`        |

Both calls take an iteration request as an input and return an iteration result as an output.
The only difference between the two, is that `helicsFederateRequestTimeIterative()` additionally takes requested time as an argument and returns granted time as an output.

## Iteration Results

There are two relevant iteration results from the calls mentioned above:

- `ITERATING`: means that _new data_ is available to the federate and it should iterate at the same time instant, or in the initialization state. In Execution mode, this result is equivalent to receiving the _same_ granted time as the previous call.

- `NEXT_STEP`: means that the federation is ready to proceed. During initialization, this means the federation can enter Execution mode, while during Execution mode this means that the federation can proceed to the next time instant.

## Iteration Request

Via the iteration request a federate tells HELICS whether their ability and desire w.r.t iteration.

- `NO_ITERATION`: forces a result of `NEXT_STEP` and makes the calling function equivalent to the version without `Iterative` in the name. **If iteration is desired, _all federates_ should avoid this request**.

- `FORCE_ITERATION`: forces a result of `ITERATING`. This should only be used in rare cases.

- `ITERATE_IF_NEEDED`: is the normal request when iteration is desired. If new data is available, this will return a result of `ITERATING`. When no new data is available it returns `NEXT_STEP` and in the case of Execution mode a new granted time.

## Guiding Principle

Iteration in HELICS is driven by new data.
If a federate does not have new input data (e.g. Subscriptions) then HELICS assumes there is no need to iterate and moves the federation forward.
As such, a critical aspect to iteration in HELICS is: **_publish first_**.
_Some_ federates must publish before calling either of the iteration function.
If _all_ federates first subscribe, there is no new data, and the simulation will move on.

This also means that the way for federates to indicate to HELICS that they no longer want to iterate is to **_not publish anything_**.

A rough outline for how iteration can be implemented using the python API is:

```python
# Time simulation loop
while grantedtime < maxtime:
    # initial publication to make sure there is new data
    h.helicsPublicationPublishDouble(pubid, pubval)

    # update requested time
    requestedtime = grantedtime + deltatime

    # Iteration loop
    while True:
        grantedtime, result = h.helicsFederateRequestTimeIterative(
            fed, requestedtime, h.helics_iteration_request_iterate_if_needed
        )
        if result == h.helics_iteration_result_next_step:
            break

        # get data
        s = h.helicsInputGetDouble(subid)

        # check convergence (e.g. any significant change on the inputs)
        if converged():
            # if converged DO NOT PUBLISH or update values
            continue

        # perform internal update
        pubval = state_update(s)

        # publish results
        h.helicsPublicationPublishDouble(pubid, pubval)
```

## Example

An example utilizing both kinds of iteration is available in [here](../examples/advanced_examples/advanced_iteration.md)
