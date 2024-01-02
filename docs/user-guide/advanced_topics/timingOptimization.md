# Introduction

Generally, most federates have both inputs they depend on and outputs on which other federates depend and configuring federates so that each one updates its own model under the correct circumstances is challenging. HELICS largely handles this through its own timing algorithm and also provides many configuration flags and settings to allow the user to optimize the timing and sequencing of federates to produce the most optimal performance. This document provides a few hints that, based on experience, we have found helpful in getting the most out of a HELICS-based co-simulation.

## `uninterruptible`

By default, a federate in HELICS is granted time when either of two conditions are met:

1. HELICS has determined it is safe for the federate to simulate its requested time based on the simulated times of all the federates from which it has inputs.
2. The federate has received an update on one of its value or message interfaces.

The latter logic is implemented under the philosophical assumption that a federate MAY want to produce a new output if one of its inputs has changed. By granting a federate an earlier time it gives the federate the ability to evaluate its new input and decide what, if any, actions it needs to take. There is some computational cost, though, to granting an earlier time as the federate must perform some kind of minimal computation to evaluate the new input. For a federate with a large number of inputs or inputs that receive changes frequently, these early time grants can add up to a relatively large amount of unnecessary computation.

To prevent this, HELICS provides for federates a flag called "uninterruptible" which removes this second option for determining the time to grant and only allows a federate to be granted the time it requests.

The `uninterruptible` flag had a significant effect on the total number of calls into the `federateRequestTime` method, the most significant source of time delay found in our testing. By reducing the number of times this method is called, especially where it may not be necessary, moderate performance gains were observed in communication-heavy federates.

Setting the `uninterruptible` via the JSON configuration is trivial:

```json
{
  "name": "sub_7",
  "period": 15,
  "logging": "warning",
  "uninterruptible": true,
  "publications": [
    {
      "global": false,
      "key": "unresponsive_mw",
      "type": "double"
    },
  ]
  "subscriptions": [
    {
      "key": "pypower/LMP_7",
      "type": "double"
    },
    {
      "key": "gld_7/distribution_load",
      "type": "complex"
    },
  ]
}
```

## Input `isUpdated`

HELICS provides a method which allows the federate to check if a specific endpoint or subscription has been updated since it was last read. In this context, "update" simply means the input has received a value since the last time it was checked. This new input may be identical in value to the last time the interface was updated and the flag simply indicates the publisher has sent another value to the subscriber's input.

If a value change is not guaranteed every time the simulator is granted time, or if multiple values are being updated in lockstep with a change on one indicating many have changed, a call to the `isUpdated` method prior to asking for the latest value can significantly reduce total computation time, and net substantial performance benefits.

```python
...
hFed = helics.helicsCreateValueFederateFromConfig(helicsConfig)
subLMP = helics.helicsFederateGetInputByTarget(hFed, tso_federate + "/LMP_" + bus)
...

# ==================== Time step looping under HELICS ===========================
helics.helicsFederateEnterExecutingMode(hFed)

while time_granted < time_stop:
    ...
    time_granted = int(helics.helicsFederateRequestTime(hFed, nextHELICSTime))
    # Faster to check than to always grab the input every time the federate
    #   is granted time
    if helics.helicsInputIsUpdated(subLMP):
        LMP = helics.helicsInputGetDouble(subLMP)
        aucObj.set_lmp(LMP)
    ...
```

## `onlyUpdateOnChange`

Similar to what the `isUpdated()` API call achieves, there is also a configuration flag that can be set to force HELICS to prevent signals from being sent (if set on the publication) or from being received (if set on the subscription) if the value of the signal has not changed. This adds a slight burden to HELICS as it now compares each published or subscribed value to the previous one to decide whether the signal will appear on the receivers interface. In cases where the computational cost of federate is large, though, this can end up being a net increase in performance.

Furthermore, a `tolerance` value can be set for each interface to define a degree of change that can be considered no-change. This allows very small changes in the published value to be ignored and treated as no change to the signal.

Configuring this in the JSON is not difficult:

```json
{
  "name": "sub_7",
  "period": 15,
  "logging": "warning",
  "publications": [
    {
      "global": false,
      "key": "unresponsive_mw",
      "type": "double",
      "onlyUpdateOnChange": true,
      "tolerance": 0.01
    },
  ]
  "subscriptions": [
    {
      "key": "pypower/LMP_7",
      "type": "double"
    },
    {
      "key": "gld_7/distribution_load",
      "type": "complex"
    },
  ]
}
```
