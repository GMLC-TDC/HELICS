# Multiple Federates on a Single Core and Managing Blocking APIs

This demonstrates the capability of HELICS to support multiple federates on a single HELICS core. Along the way, we get to put the "Async" variations of several of the HELICS APIs into action to help us manage these multiple federates in a single executable. 

- [Multiple Federates on a Single Core and Managing Blocking APIs](#multiple-federates-on-a-single-core-and-managing-blocking-apis)
  - [Where is the code?](#where-is-the-code)
  - [What is this co-simulation doing?](#what-is-this-co-simulation-doing)
    - [Differences compared to the Fundamental Default example](#differences-compared-to-the-fundamental-default-example)
      - [HELICS differences](#helics-differences)
  - [Execution and Results](#execution-and-results)
  - [Questions and Help](#questions-and-help)

## Where is the code?

The example on [using multiple federates on a single HELICS core found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_single_core). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_single_core_github.png)](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_single_core_github.png)



## What is this co-simulation doing?

This example demonstrates the HELICS capability of supporting multiple HELICS federates on a single HELICS core. A HELICS core is the connection between a HELICS federate (an instance of a simulator) and the rest of the co-simulation ((see the HELICS Terminology page for further details[../../fundamental_topics/helics_terminology]). In most cases, each federate is on its own core; this is the default behavior when creating a federate via a JSON config or the HELICS APIs. It is possible, though to have multiple federates use the same core. The HELICS auto-test suite does this commonly as a convenient way to create a federation in a single executable file. As this example shows, managing multiple federates in a single executable creates complications in managing parallel code paths. In fact, it could be argued that the point of HELICS is to help avoid the challenges of having to engage in such manual management. Nevertheless, the capabilty exists and it can be helpful in some use cases.

Additionally, this example, demonstrates the use of HELICS "Async" API calls that are useful in working around the blocking calls that are common in HELICS federates (`helicsFederateRequestTime()` being the most common). By using the "Async" variations of these API calls it is possible to further parallelize the execution of a single federation in a federation or multiple federates within an single executable.

### Differences compared to the Fundamental Default example

As compared to the Fundamental Default example, the Battery and Charger functionality have been integrated into a single executable. The physics being simulated in each federate is maintained but the order of execution within the federate code and the APIs used have been modified. There are still two federates as far as HELICS is concerned but they both use the same HELICS core. In this example, there is no federation outside of the Battery and Charger federates so the motivation to integrate the only two federates into a single executable is, uhmm, dubious. There's no reason to use HELICS if the only two modeled entities are in the same executable and can share information directly; this is an example of a capability, though, so work with me.

#### HELICS differences

The source code for the integrated Battery-Charger federate)[https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_single_core/battery_charger.py] shows that configuration is done via APIs to allow both federates to utilize the same core. To achieve this, all that is necessary is for the `federateInfo` object to have identical core names for every federate on the same core; this is achieved in this example by using the `helicsFederateInfoSetCoreName(()` API. In this example, the core name is arbitrarily chosen as "common_core". 

One of the big challenges of putting multiple federates into a single executable is managing the blocking calls in HELICS: `helicsFederateEnterInitializingMode()`, `helicsFederateEnterExecutingMode()`, `helicsFederateRequestTime()`. Any time a blocking call is made, execution halts until HELICS responds. For example, if the Battery federate in this single executable calls `helicsFederateRequestTime()`, execution halts until HELICS determines the time it can grant Battery. Unfortunately, if HELICS needs Charger to also request a time before it can determine the time to grant Battery and if the Charger time request is after Battery's', well, we have a bit of a deadlock situation. Said differently, it is often the case that both federates need to have made the time request before either of them can be granted one. 

To resolve this deadlock, HELICS provides "Async" varieties of all of these API calls. In these calls, a federate makes the API call (_e.g._ `helicsFederateRequestTimeAsync()`) and control is immediately returned to the code and no return value (the granted time) is provided. From the HELICS standpoint, it considers the time request for that federate to have been made with the granted time to be determined and communicated later. Because control returns immediately, for this example, this allows a similar time request to be made for the other federate. Depending on the configuration of the federates involved, this is generally sufficient to allow HELICS to calculate a grant time for at least one federate (though things are slightly more complicated in this example; we'll get to that).

So how does HELICS communicate the granted time to a federate that used `helicsFederateRequestTimeAsync()`? When the federate is ready to "complete" its time request it calls `helicsFederateRequestTimeComplete()`; this is a blocking call and returns the granted time. In fact, federates can also ask HELICS if a time grant has been calculated and is ready for the federate via the `helicsFederateIsAsyncOperationCompleted()`; this works for all "Async" API calls.

The "Async" versions of the API calls are useful for other reasons. Say you have a federate that after being granted time, getting the updated values on its inputs from the rest of the federation and updating its internal model (doing the "simulation" proper), it still has some bookkeeping to do: putting some values in a database or calculating some metrics for use in later post-processing. Rather than holding up the entire federation while it does this administrative work, it can use `helicsFederateRequestTimeAsync()` to make the time request, proceed to do its bookkeeping, and then call `helicsFederateRequestTimeComplete()` when the administrative tasks are complete. This allows the federation to calculate time requests and grant other federates time as soon as possible while allowing this federate in question to do its bookkeeping in parallel.

OK, back to this example. In the case of this federation, there is a further wrinkle that complicates the sequencing of the time grants: the original Fundamental Default exmaple had the Battery federate set the `wait_for_current_time_update` flag. As you may or may not know, this flag is used to indicate that this federate needs to be the last federate granted a given simulation time. That is, HELICS needs to guaranetee that all other federates have been granted a future time and thus whatever values they will produce at the simulation time in question have taken place. The use of this flag complicates the execution of these two federates in a single executable; it is my hope that the mess below is enough to make sure you need more than one federate in a single core before undertaking implementing such an endeavor. 

To achieve proper execution of the two federates with the `wait_for_current_time_update` set by the Battery federate, a complicated execution pattern must take place. Here's some psuedo-code to give you the general idea:

```
h.helicsFederateRequestTimeAsync(charger_fed, charger_requested_time)
while charger_grantedtime < total_interval:
  h.helicsFederateRequestTimeAsync(battery_fed, battery_requested_time)
  charger_grantedtime = h.helicsFederateRequestTimeComplete(charger_fed)
  charger_update_internal_model()
  charger_publish_updated_values()
  h.helicsFederateRequestTimeAsync(charger_fed, charger_requested_time)
  battery_grantedtime = h.helicsFederateRequestTimeComplete(battery_fed)
  battery_update_internal_model()
  battery_publish_updated_values()
```

As you can see in the code, Charger must update its internal model to complete its work at a given simulation time and request the next simulation time before battery is able to be granted the previous simulation time (the one Charger was just on). This means that Charger must update its internal model, publish value, and request the next time in-between Battery's `helicsFederateRequestTimeAsync()` and `helicsFederateRequestTimeComplete()`. Similar complications exist at the beginning of the execution around entering executing mode and at the end of the co-simulation when killing off the federates. 

## Execution and Results

Run the co-simulation:

```shell-session
$ helics run --path=./advanced_single_core_runner.json
```

As designed, the results of this co-simulation are identical to those of the Fundamental Default example.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultbattery.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultcharger.png)


## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
