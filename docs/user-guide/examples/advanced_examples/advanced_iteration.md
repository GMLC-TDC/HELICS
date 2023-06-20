# Iteration

This demonstrates the use of iteration both in the initialization phase, as well as execution, in order to reach convergence between federates.

- [Where is the code?](#where-is-the-code)
- [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
  - [Differences Compared to the Fundamental Examples](#differences-compared-to-the-advanced-default-example)
  - [Iteration Components](#iteration-components)
- [Execution and Results](#execution-results)
  - [Initialization Results](#initialization-results)
  - [Time Loop Results](#time-loop-results)
- [Impact of Iteration](#impact-of-iteration)

## Where is the code?

This example on [Iteration](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_iteration). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

## What is this co-simulation doing?

This example shows how to use set up the iteration calls that support state convergence across federates. This is discussed in more detail in the [User Guide](../../advanced_topics/iteration.md).

### Differences Compared to the Advanced Default Example

This example changes the model of the default example so that the charger voltage is a function of the battery current: $V_{ch}(I_{b})$. Conversely, battery current is a function of charger voltage: $I_{b}(V_{ch})$.
As a result, each time step requires some iteration to find the converged fixed-point, where both models are in a consistent state.

In very loose terms, the charging strategy uses constant current below a specific state of charge followed by constant voltage once rated voltage is achieved:
![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/charging_behavior.jpg?raw=true)
_source: <https://batteryuniversity.com/article/bu-409-charging-lithium-ion>_

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
        if EV not in [1, 2, 3]:
            out.append({"Vr": 0, "Ir": 0})
        else:
            out.append({"Vr": charge_voltages[EV - 1], "Ir": charge_currents[EV - 1]})
        logger.debug(
            "EV {} ratings: Vr = {} Ir = {}".format(EV, out[-1]["Vr"], out[-1]["Ir"])
        )
    return out
```

The charging function, $V_{ch}(I_{b})$ is implemented `voltage_update` in `Charger.py`. It considers 4 different cases (the very first `if` statement is an initialization option):

- _Case 1_: Current is within tolerance $\epsilon$ of rated current $\to$ voltage remains the same.
- _Case 2_: Current is below rated current _and_ voltage is below rated voltage $\to$ voltage is increased via bisection search.
- _Case 3_: Current is below rated _and_ voltage is rated voltage $\to$ retain rated voltage.
- _Case 4_: Current is greater than rated current $\to$ reduce voltage via bisection search.

```python
def voltage_update(
    charger_rating, charging_current, charging_voltage=None, epsilon=1e-2, quiet=False
):
    if charging_voltage is None:
        return {"V": charger_rating["Vr"], "Vmin": 0, "Vmax": charger_rating["Vr"]}
    if abs(charging_current - charger_rating["Ir"]) < epsilon:
        # Constant current charging: do nothing
        pass
    elif (charging_current < charger_rating["Ir"]) and (
        charging_voltage["V"] < charger_rating["Vr"]
    ):
        # increase voltage
        charging_voltage = {
            "V": (charging_voltage["Vmax"] + charging_voltage["V"]) / 2,
            "Vmin": charging_voltage["V"],
            "Vmax": charging_voltage["Vmax"],
        }
    elif (charging_current < charger_rating["Ir"]) and (
        abs(charging_voltage["V"] - charger_rating["Vr"]) < epsilon
    ):
        # constant voltage charging: do nothing
        pass
    elif charging_current > charger_rating["Ir"]:
        # decrease voltage
        charging_voltage = {
            "V": (charging_voltage["V"] + charging_voltage["Vmin"]) / 2,
            "Vmin": charging_voltage["Vmin"],
            "Vmax": charging_voltage["V"],
        }
    else:
        raise ValueError(
            "voltage_update: inputs do not match any of the expected cases"
        )

    return charging_voltage
```

#### Battery Model Development

In order to achieve the desired charging profile, a fictitious battery resistance $R(soc)$ is used.
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

Before diving into the details, HELICS has iteration _requests_ and _results_. The _requests_ are passed as inputs and the _results_ are returned (see the [User Guide section on iteration](../../advanced_topics/iteration.md) for further details).

The relevant _requests_ are:

- `NO_ITERATION`: this _forces_ movement to the next time step and should therefore be avoided by _all_ Federates if iteration is desired.
- `FORCE_ITERATION`: this _forces_ iteration. If a federate _always_ requests this the simulation will be stuck in a never ending loop. It is intended to be used only in _rare_ cases.
- `ITERATE_IF_NEEDED`: in this case iteration only takes place if new values are _published_. This is the _usual_ flag that should be used when iteration is desired.

The last point is **_critical_**; HELICS uses the relevant outputs of other federates to determine if a given federate needs to iterate.

The relevant _results_ are:

- `NEXT_STEP`: simulation is moving on.
- `ITERATING`: simulation is still iterating.

Each federate should therefore look for `NEXT_STEP` in order to advance in time and otherwise continue to iterate at the current time.

**Note**: An essentially equivalent method to checking the flag for `ITERATING` or `NEXT_STEP` is to check whether the currently granted time is the same the previous one.
`ITERATING` $\Rightarrow$ `grantedtime == grantedtime_previous`.
`NEXT_STEP` $\Rightarrow$ `grantedtime > grantedtime_previous`.

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

  - If the error is sufficiently small loop back and **_do not publish_**, otherwise proceed to update and publishing, which will trigger another iteration.
    The function to check the error is `check_error`:

  ```python
  def check_error(self, dState):
      return sum([abs(vals[0] - vals[1]) for vals in dState.values()])
  ```

  The function call looks like[^2]:

  ```python
  error = feditr.check_error(charging_voltage)
  if (error < epsilon) and (itr > 0):
      # no further iteration necessary
      continue
  else:
      pass
  ```

- perform state update based on subscription values.

  ```python
  # Battery.py
  charging_current[j] = current_update(charging_voltage[j][0], current_soc[j])

  # Charger.py
  charging_voltage[j] = voltage_update(
      charger_ratings[j], charging_current[j][0], charging_voltage[j]
  )
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
itr = 0
itr_flag = h.helics_iteration_request_iterate_if_needed
while True:
    grantedtime, itr_state = h.helicsFederateRequestTimeIterative(
        fed, requested_time, itr_flag
    )
    if itr_state == h.helics_iteration_result_next_step:
        break  # Iteration complete!
    else:
        pass  # Iterating
```

Similar to the initialization, it is **_critical_** to publish **_before_** the first time request, otherwise, HELICS will see no new data, and return `NEXT_STEP`.
The general simulation code is therefore:

```python
while grantedtime < total_interval:
    # Publication needed so we can actually iterate
    feditr.set_pub(fed, pubid, publication_values)

    # Time request for the next physical interval to be simulated
    requested_time = grantedtime + update_interval

    [...]
```

Note that this essentially means, that we publish our final values from time $t$ as the very first thing in time $t+\Delta t$.

## Execution Results

### Initialization Results

Two figures are produced following the initialization iteration, which show how the currents and voltages converge.
Note that all batteries are in the constant current phase of charging, as such they all converge to their rated current (30A for EV1-EV4 and 104A for EV5).
The voltages meanwhile are _not_ nominal (240V or 630V) but rather determined based on the effective impedance, which is dependent on the initialized soc.

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_current_init.png?raw=true)

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_voltage_init.png?raw=true)

### Time Loop Results

Four figures are produced once the co-simulation runs its course, two from Batter.py and two from Charger.py.

#### Battery Results

The Battery results show the the charging current in each battery, and the development of the SoC over time.
As desired, the charging current exhibits the constant current followed by a decay characteristic, and the SoC rise to 1.

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_battery_current.png?raw=true)

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_battery_SOCs.png?raw=true)

#### Charger Results

The Charger results show the how the voltage rises to its rated value and then remains there.
The total power plot is also quite interesting
Since initially the current remains fixed while the voltage rises, the total power draw increases.
Once the charging mode switches to constant voltage and the current decreases the power draw follows suite.

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_charger_voltage.png?raw=true)

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_charger_power.png?raw=true)

## Impact of Iteration

It is useful to think about _why_ iteration would be necessary in a simulation in the first place.
In a world governed by differential equations, iteration essentially smooths out timescales below the range of interest.
It allows us to assume that quicker dynamics have already settled to their steady state.
Power systems engineers might be quite familiar with this concept.
The power flow, which assumes steady state of voltage and current dependencies, requires iteration because of the use of constant power loads.
Dynamic simulations (in their simplest form), however, do not iterate, because all loads are converted to impedances.
More advanced dynamic simulations do iterate because they are attempting to account for the impact of electromagnetic phenomena (e.g. power electronics control loops) that have already settled.

In this example, the charger needs to find the appropriate voltage to achieve rated current.
In reality, there are a multitude of possible control algorithms that might be implemented.
The use of iteration here is an acknowledgement that this fixed point _will_ be found, but at a timescale that is effectively "instantaneous" w.r.t. the interval of one hour used in the simulation.
That is $\Delta t_{\text{control}} \ll \Delta t$.

To illustrate this, a second copy of the iteration is available in the repository (`Battery_noitermain.py`, `Charger_noitermain.py`, and `advanced_iteration_runner_noitermain.json`) with the iteration turn off (except for initialization) using the flag `iterative_mode = False`.
A comparison of the two runs is presented below:

|                                                                With Iteration                                                                |                                                                  Without Iteration                                                                  |
| :------------------------------------------------------------------------------------------------------------------------------------------: | :-------------------------------------------------------------------------------------------------------------------------------------------------: |
| ![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_battery_current.png?raw=true) | ![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/noiter/advanced_iteration_battery_current.png?raw=true) |
|  ![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_battery_SOCs.png?raw=true)   |  ![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/noiter/advanced_iteration_battery_SOCs.png?raw=true)   |
| ![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_charger_voltage.png?raw=true) | ![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/noiter/advanced_iteration_charger_voltage.png?raw=true) |
|  ![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/advanced_iteration_charger_power.png?raw=true)  |  ![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/iteration_example/noiter/advanced_iteration_charger_power.png?raw=true)  |

Without iteration the "voltage hunting" that happens during the constant-current charging phase takes place over hours instead of _instantaneously_ as is the case _with_ iteration.
During the constant voltage phase the two runs are identical, since there is no longer an interdependence between the federates.

Finally, it is worth noting that the oscillation observed here is a function of the convergence algorithm implemented (see function [`voltage_update`](#charger-behavior)).
A more sophisticated algorithm may lead to less oscillation.
For example, if the charger had a model of the battery SOC and its dependence on the equivalent resistance, it could estimate the current and thus obtain the right voltage immediately.
The point is, that the specific charger control architecture is not of interest in this simulation and iteration allows to abstract out the implementation without completely neglecting certain model interdependencies and focus on behavior in the time frame of interest.

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
