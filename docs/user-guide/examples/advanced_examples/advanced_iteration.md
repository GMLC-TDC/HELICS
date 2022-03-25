# Inter Timestep Iteration

This demonstrates the use of iteration both in the initialization phase, as well as execution, in order to reach convergence between federates.

- [Where is the code?](#where-is-the-code)
- [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
  - [Differences Compared to the Fundamental Examples](#differences-compared-to-the-advanced-default-example)
    - [HELICS Differences](#helics-differences)
  - [HELICS Components](#helics-components)
- [Execution and Results](#execution-and-results)

## Where is the code?

This example on [multi-inputs can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_message_comm/multi_input). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

<!-- ADD PICTURE AND LINK 
[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_multi_input_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced) -->

## What is this co-simulation doing?

This example shows how to use set up the iteration calls and determine convergence.

### Differences compared to the Default example

This example changes the model of the default example so that the charger voltage is a function of the battery current: $V_{ch}(I_{b})$. Conversely, battery current is a function of charger voltage: $I_{b}(V_{ch})$.
As a result, each time step requires some iteration to find the fixed-point, where both characteristics hold. 

In very loose terms, the charging philosophy is Constant Current followed by Constant Voltage once rated voltage is achieved:
![](https://batteryuniversity.com/img/content/new.jpg)
*source: https://batteryuniversity.com/article/bu-409-charging-lithium-ion*

#### Charger Behavior
In the default example the charger has a rated voltage and power.
This has been changed to rated voltage and current:
- _Level 1_: 120V, 15A
- _Level 2_: 240V, 30A
- _Level 3_: 630V, 104A
```python
def get_charger_ratings(EV_list):
    """
    This function uses the pre-defined charging powers and maps them to
    standard (more or less) charging voltages. This allows the charger
    to apply an appropriately modeled voltage to the EV based on the
    charging power level

    :param EV_list: Value of "1", "2", or "3" to indicate charging level
    :return: charging_voltage: List of charging voltages corresponding
            to the charging power.
    """
    out = []
    # Ignoring the difference between AC and DC voltages for this application
    charge_voltages = [120, 240, 630]
    charge_currents = [15, 30, 104]
    for EV in EV_list:
        if EV not in [1,2,3]:
            out.append({"Vr": 0 , "Ir": 0})
        else:
            out.append({"Vr": charge_voltages[EV - 1], "Ir": charge_currents[EV - 1]})
        logger.debug("EV {} ratings: Vr = {} Ir = {}".format(EV, out[-1]["Vr"], out[-1]["Ir"]))
    return out
```

The charging function, $V_{ch}(I_{b})$ is implemented `voltage_update` in `Charger.py`.
```python
def voltage_update(charger_rating, charging_current, charging_voltage=None, epsilon=1e-2, quiet=False):
    if charging_voltage is None:
        return {"V": charger_rating["Vr"], "Vmin": 0, "Vmax": charger_rating["Vr"]}
    if abs(charging_current - charger_rating["Ir"]) < epsilon:
        # Constant current charging: do nothing
        pass
    elif (charging_current < charger_rating["Ir"]) and (charging_voltage["V"] < charger_rating["Vr"]):
        # increase voltage
        charging_voltage= {"V": (charging_voltage["Vmax"] +  charging_voltage["V"])/2, 
                            "Vmin": charging_voltage["V"], 
                            "Vmax": charging_voltage["Vmax"]}
    elif (charging_current < charger_rating["Ir"]) and (abs(charging_voltage["V"] - charger_rating["Vr"]) < epsilon):
        # constant voltage charging: do nothing
        pass
    elif charging_current > charger_rating["Ir"]:
        # decrease voltage
        charging_voltage = {"V": (charging_voltage["V"] + charging_voltage["Vmin"])/2,
                                "Vmin": charging_voltage["Vmin"],
                                "Vmax": charging_voltage["V"]}
    else:
        raise ValueError("voltage_update: inputs do not match any of the expected cases")
        
    return charging_voltage
```

It considers 4 different cases (the very first `if` statement is an initialization option):
- _Case 1_: Current is within tolerance $\epsilon$ of rated current $\to$ voltage remains the same.
- _Case 2_: Current is below rated current _and_ voltage is below rated voltage $\to$ voltage is increased via bisection search.
- _Case 3_: Current is below rated _and_ voltage is rated voltage $\to$ retain rated voltage.
- _Case 4_: Current is greater than rated current $\to$ reduce voltage via bisection search.

#### Battery Behavior
In order to achieve the desired charging profile a fictitious battery resistance $R(soc)$ is used.
As this $R$ increases the voltage must increase to maintain the rated current $I_{\text{rated}}$ until $V_{\text{rated}}$ is reached and held, at which point the current begins to taper.

We utilize the following design criteria:
1. The actual internal resistance of a battery is about on the order of 0.5 $\Omega$[^1], which will be used as the $R(soc=0)$
2. We would like constant current charging until around $soc=0.6$.
3. Charging is considered full when current drops below 3% rated.

[^1]: [See this article](https://batteryuniversity.com/article/how-does-internal-resistance-affect-performance#:~:text=The%20internal%20resistance%20of%20lithium,acid%20goes%20up%20with%20discharge)

From Criteria 2 we have $R(0.6) = V_{\text{rated}}/I_{\text{rated}}$. 
For the three charger types we have defined that is:
- _Level 1_: $R(0.6) = 120\text{V}/15\text{A} = 8 \Omega $
- _Level 2_: $R(0.6) = 240\text{V}/30\text{A} = 8 \Omega $
- _Level 3_: $R(0.6) = 630\text{V}/104\text{A} \approx 6 \Omega $

Splitting the difference we fix $R(0.6) \overset{!}{=} 7\Omega$.

From Criteria 3 we have $R(1) = V_{\text{rated}}/(0.03\cdot I_{\text{rated}})$.
For the three charger types defined that is:
- _Level 1_: $R(0.6) = 120\text{V}/(0.03\cdot 15\text{A} )\approx 267 \Omega $
- _Level 2_: $R(0.6) = 240\text{V}/(0.03\cdot 30\text{A} )\approx 267 \Omega $
- _Level 3_: $R(0.6) = 630\text{V}/(0.03\cdot 104\text{A}) \approx 202 \Omega $

Taking the maximum (so we make sure the current decays _at least_ to 3% rated by full charge) we fix $R(1) \overset{!}{=} 267\Omega$.

the function $R(soc)$ is now created as a two linear segments: one from 0 to 0.6 and the other from 0.6 to 1:
```python
def effective_R(soc):
    if soc >= 0.6:
        return 650 * soc - 383
    else:
        return 10.83 * soc + 0.5
```

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/R_of_SOC.png?raw=true)

The current is then simply:
```python
def current_update(charging_voltage, soc):
    # Calculate charging current and update SOC
    R = effective_R(soc)
    # If battery is full assume its stops charging on its own
    #  and the charging current goes to zero.
    if soc >= 1:
        return 0
    else:
        return max(0, charging_voltage / R)
```

### Iteration components
The majority of the iteration related code is found in `iterutils.py` in order to reuse it in both `Battery.py` and `Charger.py`. 

Before diving into the details, HELICS has iteration _requests_ and _results_.
The _requests_ are passed as inputs and the _results_ are returned.

The relevant _requests_ are:
- `NO_ITERATION`: this _forces_ movement to the next time step and should therefore be avoided by _all_ Federates if iteration is desired.
- `FORCE_ITERATION`: this _forces_ iteration. If a federate _always_ requests this the simulation will be stuck in a never ending loop.
- `ITERATE_IF_NEEDED`: in this case iteration only takes place if new values are _published_.

The last point is **_critical_**.
The signal HELICS is looking for, is that no new values are published.

The relevant _results_ are:
- `NEXT_STEP`: simulation is moving on.
- `ITERATING`: simulation is still iterating.

Each federate should therefore look for `NEXT_STEP` in order to continue.

#### Initialization
The first step is to initialize the federates.
In this step `helicsFederateEnterInitializingMode` is called and the federates publish their initial values.

The function is `set_pub` (with option `init=True` and logging commands taken out for clarity):
```python
def set_pub(self, fed, pubid, pubvals, nametyp=None, init=False):
    if init:
        self.logger.info("=== Entering HELICS Initialization mode")
        h.helicsFederateEnterInitializingMode(fed)
    pub_count = h.helicsFederateGetPublicationCount(fed)
    for j in range(0, pub_count):
        h.helicsPublicationPublishDouble(pubid[j], pubvals[j])
```

The function call looks like:
```python
# Battery.py
feditr.set_pub(fed, pubid, charging_current, "Battery", init=True)

# Charger.py
feditr.set_pub(fed, pubid, [x["V"] for x in charging_voltage], "EV", init=True)
```

Next, the federates _iterate_ using `helicsFederateEnterExecutingModeIterative`.
The basic structure is:
- Call `helicsFederateEnterExecutingModeIterative`.
  - If the result is `NEXT_STEP` then we're done.
  ```python
  itr = 0
  itr_flag = h.helics_iteration_request_iterate_if_needed
  while True:
    itr_status = h.helicsFederateEnterExecutingModeIterative(fed, itr_flag)
    if itr_status == h.helics_iteration_result_next_step:
        break
  ```
- Get subscriptions. Done via the `get_sub` function:
  ```python
  def get_sub(self, fed, subid, itr, valarray, valinit, nametyp, proptyp):
      sub_count = h.helicsFederateGetInputCount(fed)
      for j in range(0, sub_count):
          x = h.helicsInputGetDouble((subid[j]))
          if itr == 0:
              valarray[j] = [valinit] * 2
          else:
              valarray[j].insert(0, valarray[j].pop())
          valarray[j][0] = x
  ```
  Here `valarray` stores the current _and_ previous value to check for convergence.
  The function call looks like:
  ```python
  # Battery.py
  feditr.get_sub(fed, subid, itr, charging_voltage, vinit, "Battery", "voltage")
  
  # Charger.py
  feditr.get_sub(fed, subid, itr, charging_current, iinit, "EV", "current")
  ``` 
- check error
  - If the error is sufficiently small loop back and **_do not publish_**, othewise force iteration and continue
  The function to check the error is ``check_error``:
  ```python
  def check_error(self, dState):
        return sum([abs(vals[0] - vals[1]) for vals in dState.values()])
  ```
  The function call looks like[^2]:
  ```python
  error = feditr.check_error(charging_voltage)
  if (error < epsilon) and (itr > 0):
      # no further iteration necessary
      itr_flag = h.helics_iteration_request_iterate_if_needed
      continue
  else:
      itr_flag = h.helics_iteration_request_force_iteration
      pass
  ```
- perform state update based on subscription values.
  ```python
  # Battery.py
  charging_current[j] = current_update(charging_voltage[j][0], current_soc[j])

  # Charger.py
  charging_voltage[j] = voltage_update(charger_ratings[j], charging_current[j][0], charging_voltage[j])
  ```
- publish new outputs and increment iteration
  ```python
  # Battery.py
  feditr.set_pub(fed, pubid, charging_current)
  itr += 1

  # Charger.py
  feditr.set_pub(fed, pubid, [x["V"] for x in charging_voltage])
  itr += 1
  ```
  
[^2]: For `Battery.py` in `Charger.py` the input is `charging_current` instead of `charging_voltage`.

#### Time Loop
The time loop looks almost identical to the initialization loop, except that instead of calling `helicsFederateEnterExecutingModeIterative`,
the call is to `helicsFederateRequestTimeIterative`:
```python
while True:
  grantedtime, itr_state = h.helicsFederateRequestTimeIterative(fed, requested_time, itr_flag)
  if grantedtime < requested_time:
      itr_flag = h.helics_iteration_request_iterate_if_needed
      continue
  elif itr_state == h.helics_iteration_result_next_step:
      break
```
In this case we don't want to take any action if the granted time is smaller than the requested time.
In other situations this _could_ be different.
The underlying assumption here is that any changes between time requests are effectively "too fast" to change the output.

## Execution Results
### Initialization
Two figures are produced following the initialization iteration, which show how the currents and voltages converge.
Note that all batteries are in the constant current phase of charging, as such they all converge to their rated current (30A for EV1-4 and 104A for EV5).
THe voltages meanwhile are _not_ nominal (240V or 630V) but rather determined based on the effective impedance, which is dependent on the initialized soc.

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_current_init.png?raw=true)

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_voltage_init.png?raw=true)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](mailto:helicsteam@helics.org)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!