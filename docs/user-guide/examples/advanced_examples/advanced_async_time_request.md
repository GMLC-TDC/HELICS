# Asynchronous (non-blocking) Time Request

Generally, when federates make a time request they do so by calling `helicsFederateRequestTime()`. This is a blocking call; the HELICS library will wait until it has reached the time it determines should be granted to the federate and then will do so, returning the granted time. In many cases, this blocking call is acceptable as a time request generally means a federate has nothing to do until it receives new inputs from the rest of the federation as it has completed updating its internal model based on the last received inputs from the federation.

There are cases, though, where a federate may want to make a time request and then continue working on another computational task instead of waiting in an idle state for the time request to be granted. HELICS provides an alternative time request API to accomplish this: `helicsFederateRequestTimeAsync()`.

## Using the Asynchronous Time Request

Making the asynchronous time request is simple: to make the non-blocking time request use the `helicsFederateRequestTimeAsync()` API. This API does not return a value and does not block the federate operation. The code following this API will begin immediately executing while the rest of the federation advances through simulated time. When this federate is done with its extra work, it uses the `helicsFederateRequestTimeComplete()` API call. This is a blocking call (just like `hliecsFederateRequestTime()` was), though it is possible the state of the federation is such that HELICS is ready to immediately grant a time.

There is also a similar set of APIs when working with the iterative time requests: `helicsFederateRequestTimeIterativeAsync()` and `helicsFederateRequestTimeIterativeComplete()`.

## Example Explanation

The advanced default example has been very slightly modified to demonstrate the use of `helicsFederateRequestTimeAsync()` and `helicsFederateRequestTimeComplete()`. In this case, Battery.py has been edited to always make the async time request and then immediately call a frivolous function that, once a simulated day, delays the time request by one wall-clock second and sends a message to the logs. (Obviously, in a real-world application the work done in this function would not be frivolous.) Running this example and looking at the logs shows this message, indicating the function is being run after the async time request but before completing the time request.

The results of this example are identical to those of the Advanced Default example as there is no impact on the simulated results, just the wall-clock time it takes to run the simulation.

## Running the example

The example can be found in the [HELICS Examples repopsitory.](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_async_time_request)

To run the example:

```shell
$ helics run --path=advanced_async_runner.json
```

And the results should look like this:
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_estimated_SOCs.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_default_charging_power.png)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
