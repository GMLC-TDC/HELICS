# Iteration

In simulation in general, iteration at a time step is can be helpful in the case of [algebraic loops](https://www.mathworks.com/help/simulink/ug/algebraic-loops.html), that is, when two state variables are co-dependent on each other. If the analysis requires the two sub-systems to reach a consistent state, then iteration is required. In these situation, different methods (generally Fixed-Point Methods, like Newton's method), can be used to arrive at the desired solution. When modeling such systems in a single executable a traditional solver can be used as all the system states are visible to said solver. When modeling these systems in a co-simulation environment, no single solver has the necessary visibility to force convergence between the two sets of states variables and iteration must instead by facilitated by HELICS. Iterations at a single time step are intended to resolve this issue by allowing Federates to exchange data back and forth until convergence to a consistent state is reached.

```{note}
Since neither federate has a full view of the problem, convergence issues cannot be dealt with quite in the same way as general numerical solvers. In fact, convergence cannot be guaranteed, because no federate really knows the logic/trajectory of the state variables in the other federates it is iterating against.
```

## Simulation Tool Support for Iteration

Though HELICS provides specific APIs to iterate between federates, the underlying tools may or may not support such iteration. Generally speaking, if the models inside the simulation tools are stateless (that is, only dependent on the current input to produce a new output) then iteration at a given time will be possible. As other federates iterate and produce new outputs, a stateless simulator can take those as its new inputs and likewise produce new outputs.

Many simulation tools, though, do not fit into this category. The use of previous system states to find the current state of the model is very common. For example, if the model of the system includes differential equations, at least one previous state will be required to iterate at a fixed time. Not all simulation tools appropriately store these previous states and is is the job of the person integrating the simulation tool to determine if iteration is supported by the tool in question and use the HELICS APIs appropriately.

## HELICS APIs

There are two possible co-simulation states where iterations are possible in HELICS:

| State            | Iteration call                                |
| ---------------- | --------------------------------------------- |
| `INITIALIZATION` | `helicsFederateEnterExecutingModeIterative()` |
| `EXECUTION`      | `helicsFederateRequestTimeIterative()`        |

Both calls take an `iteration_request` as an input and return an `iteration_result` as an output. The only difference between the two, is that `helicsFederateRequestTimeIterative()` additionally takes requested time as an argument and additionally returns granted time as an output alongside `iteration_result`. If iterating in execution mode, it is necessary that the requested time be the next time appropriate for the federate (as if you were not iterateing). Requesting the same simulated time a federate is at AND making it an iterative time request is a recipe for HELICS disaster; don't do it.

### `iteration_request`

When using the iteration APIs, the `iteration_request` parameter of the call indicates to HELICS what what the federate's ability and desire is when iterating.

- `NO_ITERATION`: forces a result of `NEXT_STEP` and is equivalent to not using the iterative APIs at all. Using this value for `iteration_request`, `NO_ITERATION` effectively opts the given federate out of the iteration at the given timestep, allowing other federates to continue iterating if they so desire. This can be useful if a given federate only needs to be involved in iteration under specific conditions.

- `FORCE_ITERATION`: forces `iteration_result` for all iterating federates to be `ITERATING`. This should only be used in rare cases.

- `ITERATE_IF_NEEDED`: is the normal request when iteration is desired. This could have been called `ITERATE_IF_NEW_INPUTS`; if new data is available to the federate making this request, `iteration_result` will be `ITERATING`. When no new data is available it returns `NEXT_STEP` and in the case of Execution mode, a new granted time is also returned. Return values of `NEXT_STEP` will always be accompanied with granted times that move the simulation forward.

### `iteration_result`

There are two relevant iteration results from the calls mentioned above:

- `ITERATING`: means that _new data_ is available to the federate and it should could iterating at the current simulated time. When the co-simulation is in Execution mode, this result is equivalent to receiving the _same_ granted time as the previous call.

- `NEXT_STEP`: means that the federation is ready to proceed. During initialization, this means the federation can enter Execution mode, while during Execution mode this means that the federation can proceed to the next time instant.

## Convergence Criterion Definition

A core tenent of HELICS co-simulation philosophy is local autonomy with distributed control. That is, no federate can force another federate to do something. For co-simulation to work at all, the federates have to choose to cooperate (obviously) but each federate is responsible only for itself and does not rely on external federates to manage its own operation. When it comes to iteration, this can get a bit tricky.

Traditional integrated simulation tools with iterative solvers generally have access to all the information necessary and can use a variety of convergence criterion when deciding when to iterate and when to move on. In a co-simulation environment, there is no central authority for convergence and it is the obligation of each federate to appropriately implement its own convergence criterion. It is also entirely possible that these criterion are different across federates and, if that is the case, there is no guarantee that any given federate's convergence criterion will be met. (Generally, only the loosest criterion can be guaranteed to be satisfied.)

Because federates don't generally expose all of their internal states (just those that other federates need for a particular analysis or use case), it is highly recommended that each federate base its own convergence criterion on received changes in inputs. That is, each federate should be evaluating how much its subscribed values are changing from iteration to iteration and when those inputs have settled down and are changing sufficiently little from iteration to iteration, the federate should consider itself converged. At this point, it has been demonstrated that changes in the federates outputs are having negligible impact on the rest of the federation and it is likely the federation as a whole has convereged.

## Guiding Principles

### Publish Before Iterative Time Requests

Iteration in HELICS is driven by the presence of new data produced by other members of federation. If a federate does not have new input data from the federations then HELICS assumes there is no need to iterate and will return `NEXT_STEP` from the iterating call. Because of this, a critical aspect to iteration in HELICS is: **_publish before making the iterating call_**. This will ensure all other federates relying on these inputs will have them and know they need to iterate. Producing new outputs effectively forces all federates that have requested `ITERATE_IF_NEEDED` to iterate once more.

### Indicate Convergence By Not Publishing Before Iterative Time Requests

The big and important exception to this is if a given federate has reached a state where it considers itself converged. At this point, the only thing the federate needs to do to indicate this to the rest of the federation is not publish any new values before making the exact same iterative time request. That iterative time request should still use `ITERATE_IF_NEEDED` and if it is returned `ITERATING` it should look for new inputs. If the convergence criterion is met, it once again would not publish anything and make the same request again. If the convergence criterion is no longer met it should recalculate the model state and publish new outputs.

### `only_update_on_change`

Since iteration is highly dependent on received inputs, an implicit convergence criterion is implemented through the [`only_update_on_change`](../../references/configuration_options_reference) flag. When set this flag will only show new inputs to federates if they have changed since the last time they have been published. This can be helpful in that if there is a slightly mis-behaving federate that published every time, regardless of whether it thinks it has converged, those republications can be screened out. Additionally, `only_update_on_change` has a related parameter called [`tolerance`](../../references/configuration_options_reference.md#tolerance-0) that allows values within a certain numerical range from the previously published value to be considered "unchanged" and will not be presented to the federate. This can effectively be used to define the convergence criterion as it limits the changes in inputs to a federate.

## Example Iterative Federate Psuedo-code

A rough outline for how iteration can be implemented using the python API is:

```python
# Time simulation loop
while grantedtime < maxtime:
    # initial publication to make sure there is new data
    h.helicsPublicationPublishDouble(pubid, pubval)

    # update requested time
    requestedtime = grantedtime + deltatime

    converged = false

    # Iteration loop
    while not converged:
        grantedtime, iterative_return = h.helicsFederateRequestTimeIterative(
            fed, requestedtime, h.helics_iteration_request_iterate_if_needed
        )
        # If the iterative call is telling us to move on, we can break out
        #  of the iterative while loop and move on in time
        if iterative_return == h.helics_iteration_result_next_step:
            break

        # Save old input states and get new one
        s_old = s
        s = h.helicsInputGetDouble(subid)

        # check convergence (e.g. any significant change on the inputs)
        converged = check_convergence(s, s_old)

        if not converged():
            # perform internal update
            pubval = state_update(s)

            # publish results
            h.helicsPublicationPublishDouble(pubid, pubval)
```

## Example

An example utilizing both kinds of iteration is available in [here](../examples/advanced_examples/advanced_iteration.md)
