# Brokers - Simultaneous Co-simulations

This example shows how to configure a HELICS co-simulation so that multiple co-simulations can run simultaneously on one computer. Understanding this configuration is a pre-requisite to running the other advanced broker examples (which also involve multiple brokers running on one computer).

- [Where is the code?](#where-is-the-code)
- [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
  - [Differences Compared to the Fundamental Examples](#differences-compared-to-the-advanced-default-example)
    - [HELICS Differences](#helics-differences)
    - [Research Question Complexity Differences](#research-question-complexity-differences)
- [Execution and Results](#execution-and-results)

## Where is the code?

The code for the [Advanced examples](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced) can be found in the HELICS-Examples repository on GitHub. This example on [simultaneous co-simulations can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_brokers/simultaneous). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_simultaneous_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced)

## What is this co-simulation doing?

Using the exact same federates as in the [Advanced Default example](./advanced_default.md), the same co-simulation is run multiple times (simultaneously) with different random number generator seeds. The example both demonstrates how to run multiple HELICS co-simulations simultaneously on one computer without the messages between federates getting mixed up, but also shows a simple way to do sensitivity analysis. A better way is shown later in the [orchestration example.](./advanced_orchestration.md)

### Differences compared to the advanced default example

Two primary changes:

1. This example contains a set of co-simulations with each instance using a different random number generator seed in Battery.py

```python
if __name__ == "__main__":
    np.random.seed(2608)
```

and Charger.py

```python
if __name__ == "__main__":
    np.random.seed(1490)
```

The values shown above are from [federation_1](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_brokers/simultaneous/federation_1). Identical lines with alternative values can be found in [federation_2](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_brokers/simultaneous/federation_2) and [federation_3.](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_brokers/simultaneous/federation_3)

2. The brokers are configured to ensure that messages from one federation do not get routed to federates in another federation.

#### HELICS differences

With no extra configuration, it is only possible to run one HELICS co-simulation on a given computer. In the most popular HELICS cores (ZMQ being the most common, by far), messages are sent between federates using the networking stack. (There are other ways, though. For example, the IPC core uses the Boost library inter-process communication.). If you want to run multiple co-simulations on one compute need, an extra step needs to be taken to keep the messages from each federation separate from each other and non-interfering. Since we're using the network stack, this can be easily accomplished by assigning each broker a unique port to use. Looking at the federation launch config files, you can see this clearly expressed:

[federation_1_runner.json](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_brokers/simultaneous/federation_1/federation_1_runner.json)

```json
{
  "federates": [
    {
      "directory": ".",
      "exec": "helics_broker -f 3 --loglevel=1 --port=20100",
      "host": "localhost",
      "name": "broker"
    },
```

[federation_2_runner.json](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_brokers/simultaneous/federation_2/federation_2_runner.json)

```json
{
  "federates": [
    {
      "directory": ".",
      "exec": "helics_broker -f 3 --loglevel=1 --port=20200",
      "host": "localhost",
      "name": "broker"
    },
```

[federation_3_runner.json](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_brokers/simultaneous/federation_3/federation_3_runner.json)

```json
{
  "federates": [
    {
      "directory": ".",
      "exec": "helics_broker -f 3 --loglevel=1 --port=20300",
      "host": "localhost",
      "name": "broker"
    },
```

#### Research question complexity differences

The Advanced Default example uses a random number generator to determine things like initial state-of-charge of the battery and charging power of the individual EV batteries. These factors have an impact on the charging duration for each EV battery and the peak charging power seen over the duration of the simulation. Since the later is _the_ key metric of the simulation experiment, there is strong motivation to vary the seed value for the random number generator to expand the range of results, effectively increasing the sample size. These co-simulations could each be run serially but assuming the computer in question has the horsepower, there's no reason not to run them in parallel.

## Execution and Results

To run the co-simulations simultaneously, all that is required is having the HELICS runner launch each individually. The trailing `&` in the shell commands below background the command and return another shell prompt to the user.

`$ helics run --path=./federation_1/federation_1_runner.json &`
`$ helics run --path=./federation_2/federation_2_runner.json &`
`$ helics run --path=./federation_2/federation_3_runner.json &`

The peak charging results are shown below. As can be seen, the peak power amplitude and the total time at peak power are impacted by the random number generator seed.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_simultaneous_power_1.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_simultaneous_power_2.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_simultaneous_power_3.png)

To do a more legitimate sensitivity analysis to the population of EVs that are being charged, a sample size larger than three is almost certainly necessary. [We've put together another example](./advanced_orchestration.md) to show how to orchestrate running larger sets of co-simulations to address exactly these kinds of needs.

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
