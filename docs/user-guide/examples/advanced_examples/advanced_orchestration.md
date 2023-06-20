<!-- identify two options for testing this example:
1. manual
2. Merlin spec for Cosimulation
 -->

# Monte Carlo Co-Simulations

This tutorial will walk through how to set up a HELICS Monte Carlo simulation using two techniques: (1) in series on a single machine, and (2) in parallel on an HPC cluster using Merlin. We assume that you have already completed the
[**orchestration tutorial with Merlin**](../../advanced_topics/orchestration.md) and have some
familiarity with how Merlin works.

- [Where is the code?](#where-is-the-code)
- [What is this Co-simulation doing?](#what-is-this-co-simulation-doing)
- [Probabilistic Uncertainty Estimation](#probabilistic-uncertainty-estimation)
- [Execution and Results](#execution-and-results)
  - [Manual Orchestration Execution](#manual-orchestration-execution)
  - [Merlin Orchestration Execution](#merlin-orchestration-execution)

## Where is the code?

Code for the Monte Carlo simulation and the
full Merlin spec and be found in the [HELICS Examples Repo](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_orchestration). If you have issues navigating the examples, visit the HELICS [Gitter page](https://gitter.im/GMLC-TDC/HELICS) or the [user forum on GitHub](https://github.com/GMLC-TDC/HELICS/discussions).

[![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/advanced_orchestration_github.png)](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_orchestration)

The necessary files are:

- Python program for Battery federate ([Battery.py](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_orchestration/Battery.py))
- Python program for Charger federate ([Charger.py](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_orchestration/Charger.py))
- Python program to generate HELICS runner JSON files and execute ([advanced_orchestration.py](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/advanced/advanced_orchestration/advanced_orchestration.py))

## What is this co-simulation doing?

This example walks through how to set up a probabilistic model with Monte Carlo simulations. This Monte Carlo co-simulation is built from a simple two federate example, based on the [Endpoint Federates Example](../fundamental_examples/fundamental_endpoints.md). In this example, there is a Charger federate which publishes voltage and a Battery federate which publishes current.

All of the HELICS configurations are the same as in the Endpoint example. The internal logic of the federates has been changed for this implementation. The Charger federate assumes the role of _deciding_ if the Battery should continue to charge. The Battery sends a message of its current SOC (state of charge), a number between 0 and 1. If the SOC is less than 0.9, the Battery is instructed to continue charging, otherwise, it is instructed to cease charging. The Battery federate has all the logic internal for adding energy and selecting a new "battery" (charging rate) if the SOC is deemed sufficient. Energy is added to the "battery" according to the previous time interval and the charge rate of the battery. In this way, the only stochastic component to the system is the **selected charge rate**. For example, the Endpoint Example allowed the Battery federate to randomly select batteries of different sizes, and the Charger to select charge rates from a list of options. In this implementation, the battery size (capacity in kWh) is constant.

This simplification allows us to isolate a single source of uncertainty: the charge rate.

The co-simulation relies on stochastic sampling of distributions -- an initial selection of vehicles for the EV charging garage. We want to ensure that we are not overly reliant on any one iteration of the co-simulation. To manage this, we can run the co-simulation _N_ times, or a Monte Carlo co-simulation. The result will be a _posterior distribution_ of the instantaneous power draw over a desired period of time.

## Probabilistic Uncertainty Estimation

A Monte Carlo simulation allows a researcher to sample random numbers repeatedly from a predefined distribution to explore and quantify uncertainty in their analysis. Additional detail about Monte Carlo methods can be found on [Wikipedia](https://en.wikipedia.org/wiki/Monte_Carlo_method) and [MIT Open Courses](https://www.youtube.com/watch?v=OgO1gpXSUzU).

In a Monte Carlo co-simulation, a probability distribution of possible values can be used in the place of **any** static value in **any** of the simulators. For example, a co-simulation may include a simulator (federate) which measures the voltage across a distribution transformer. We can quantify measurement error by replacing the deterministic (static) value of the measurement with a random value from a uniform distribution. Probabilistic distributions are typically described with the following notation:

_M ~ U(a,b)_

Where _M_ is the measured voltage, _a_ is the lower bound for possible values, and _b_ is the upper bound for possible values. This is read as, "_M_
is distributed uniformly with bounds _a_ and _b_."

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/uniform_dist.png)

The uniform distribution is among the most simple of probability distributions. Additional resources on probability and statistics are plentiful; [Statistical Rethinking](https://xcelab.net/rm/statistical-rethinking/) is highly recommended.

### Monte Carlo Co-sim Example: EV Garage Charging

The example co-simulation to demonstrate Monte Carlo distribution sampling is that of an electric vehicle (EV) charging garage. Imagine a parking garage that only serves EVs, has a static number of charging ports, and always has an EV connected to a charging port. An electrical engineer planning to upgrade the distribution transformer prior to building the garage may ask the question: What is the likely power draw that EVs will demand?

#### Probability Distributions

_Likely_ is synonymous for _probability_. As we are interested in a probability, we cannot rely on a deterministic framework for modeling the power draw from EVs. I.e., we cannot assume that we know a priori the exact demand for Level 1, Level 2, and Level 3 chargers in the garage. A deterministic assumption would be equivalent to stating, e.g., that 30% of customers will need Level 1 charge ports, 50% will need Level 2, and 20% will need Level 3. What if, instead of static proportions, we assign a distribution to the need for each level of charge port. The number of each level port is discrete (there can't be 0.23 charge ports), and we want the number to be positive (no negative charge ports), so we will use the [Poisson distribution](https://en.wikipedia.org/wiki/Poisson_distribution). The Poisson distribution is a function of the anticipated average of the value _λ_ and the number of samples _k_. Then we can write the distribution for the number of chargers in each level, _L_, as

_L ~ P(k,λ)_

Let's extend our original assumption that the distribution of chargers is static to Poisson distributed, and let's assume that there are 100 total charging ports:

_L1 ~ P(100,0.3)_

_L2 ~ P(100,0.5)_

_L3 ~ P(100,0.2)_

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/EVPoisson.png)

What if we weren't entirely certain that the average values for _L1, L2, L3_ are _0.3, 0.5, 0.2_, we can also sample the averages from a normal distribution centered on these values with reasonable standard deviations. We can say that:

_λ ~ N(μ,σ)_

Which means that the input to _L_ is distributed normally with average _μ_ and standard deviation _σ_.

Our final distribution for modeling the anticipated need for each level of charging port in our _k = 100_ EV garage can be written as:

_L ~ P(k,λ)_

_λ ~ N(μ,σ)_

<center>

|     | _L1_     |   _L2_   |           _L3_ |
| --- | :------- | :------: | -------------: |
| _μ_ | 0.3      |   0.5    |            0.2 |
| _σ_ | ~ N(1,3) | ~ N(1,2) | ~ N(0.05,0.25) |

</center>

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/EVfulldist.png)

Notice that the individual overplotted distributions in the two histograms above are different -- there is more flexibility encoded into the second. The distributions in the second plot describe the following assumptions about the anticipated need for Level 1, 2, and 3 chargers:

1. The number of charging ports is discrete and positive (Poisson).
2. The variance in the number of ports should be normally distributed.
3. The average percentage of Level 1, 2, and 3 chargers is 30%, 50%, and 20%, respectively.
4. The variance around the average is uniformly distributed.
5. The variance is relatively very broad for Level 1, broad for Level 2, and very constrained for Level 3. This means we are very confident in our guess of the average percentage for Level 3 chargers, less confident for Level 2, and much less confident for Level 3.

We have described the individual distributions for each level of charging port. What we don't know, prior to running the Monte Carlo co-simulation, is how these distributions will jointly impact the research question.

#### Research Question Configuration

We want to address the research question: What is the likely power draw that EVs will demand?

At the beginning of the co-simulation, the distributions defined above will be sampled _N_ times within the EV federate, where _N =_ the number of parking spots/charging ports in the garage. The output of the initial sampling is the number of requested Level 1, 2, and 3 charging ports. The SOC for the batteries on board are initialized to a uniform random number between 0.05 and 0.5, and these SOC are sent to the Charger federate. If the SOC of an EV battery is less than 0.9, the EV Controller federate tells the EV battery in the EV federate to continue charging. Otherwise, the EV Charger disconnects the EV battery, and instructs the it to sample a new EV battery from the distributions (one sample).

After the two federates pass information between each other -- EV Battery sends SOC, EV Charger instructs whether to keep charging or resample the distributions -- the EV Battery calculates the total power demanded in the last time interval.

## Execution and Results

Execution can be done with either a simple script (provided on the repo), or with Merlin.

### Manual Orchestration Execution

Manual implementation of the co-simulation is done with the helper script `advanced_orchestration.py`, with command line execution:

```shell session
$ python advanced_orchestration.py
```

This implementation will run a default co-simulation. The default parameters are:

```python
samples = 30
output_path = os.getcwd()
numEVs = 10
hours = 24
plot = 0
run = 1
```

This means that we are generating 30 JSON files with unique seeds, we are using the current operating directory as the head for the output path, we are simulating 10 EVs in the co-simulation for one day, we are not running individual plots for each simulation, and we are executing the JSON files with the HELICS runner after they have been created.

If we wanted to run a Monte Carlo co-sim with different parameters, this would be:

```shell session
$ python advanced_orchestration.py 10 . 100 24*7 0 0
```

This execution would create 10 JSON files with unique seeds, set the current directory as the head for the output path, simulate 100 EVs for a week, not generate plots with each simulation, and not execute the JSON files with the HELICS runner (meaning the user will need to manually run each JSON file).

You may decide to adapt `advanced_orchestration.py` to suite your needs within the Merlin environment, in which case you would only need the helper script to create the JSON files. If you elect to use the HELICS runner for the generated runner JSON files using the helper script, subdirectories are created for the HELICS runner JSON files and for the csv results. Results for the default simulation are in the repo and can be used for confirming accurate execution.

```python
out_json = output_path + "/cli_runner_scripts"
out_data = output_path + "/results"
```

In the runner scripts directory, there will be 30 JSON files. Each will have a unique `seed` parameter, otherwise they will all look identical:

```json
{
  "federates": [
    {
      "directory": "[path to local HELICS-Examples/user_guide_examples/advanced/advanced_orchestration dir]",
      "exec": "helics_broker --federates=2 --loglevel=data --coretype=tcpss --port 12345",
      "host": "localhost",
      "loglevel": "data",
      "name": "broker_0"
    },
    {
      "directory": "[path to local HELICS-Examples/user_guide_examples/advanced/advanced_orchestration dir]",
      "exec": "python3 Battery.py --port 12345 --seed 10 --numEVs 10 --hours 24 --plot 0 --outdir [path to local HELICS-Examples/user_guide_examples/advanced/advanced_orchestration dir]/results",
      "host": "localhost",
      "loglevel": "data",
      "name": "Battery_0"
    },
    {
      "directory": "[path to local HELICS-Examples/user_guide_examples/advanced/advanced_orchestration dir]",
      "exec": "python3 Charger.py --port 12345 --numEVs 10 --hours 24",
      "host": "localhost",
      "loglevel": "data",
      "name": "Charger_0"
    }
  ],
  "name": "Generated by advanced orchestration"
}
```

The final result of the default Monte Carlo co-simulation is shown below.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/MonteCarlo_Manual_small.png)

This is a time series density plot. Each simulation is a green line, and the blue solid line is the median of all simulations. From this plot, we can see that (after the system [initializes](../../fundamental_topics/stages.md#initialization), after a few hours) the maximum demand from EVs in the garage will be roughly 125 kW. We could improve the analysis by conducting an initialization step and by running the simulation for a longer time period. This type of analysis provides the engineer with information about the probability that demand for power from N EVs will be X kW. The most commonly demanded power is less than 50 kW -- does the engineer want to size the power conduit to provide median power, or maximum power?

### Merlin Orchestration Execution

In this specification we will be using the
HELICS runner to execute each co-simulation run since this is a Monte Carlo simulation. This means that the HELICS runner will be called multiple times with different JSON runner files.

#### Co-simulation Reproduction

Management of multiple iterations of the co-simulation can be done by setting the seed as a function of the number of brokers, where there will be one broker for each iteration. In Python 3.X:

```python
import argparse

parser = argparse.ArgumentParser(description="EV simulator")
parser.add_argument(
    "--seed",
    type=int,
    default=0,
    help="The seed that will be used for our random distribution",
)
parser.add_argument("--port", type=int, default=-1, help="port of the HELICS broker")


args = parser.parse_args()
np.random.seed(args.seed)
```

#### HELICS runner in Merlin

Since we are using the HELICS runner to manage and execute all the
federates, we need to create these runner JSON files.
There is a provided python script called `make_samples_merlin.py` (see the
`simple` subfolder in the code for the example) that will generate the
runner file and a csv file that will be used in the study step. The HELICS runner command will start each of these federates. In the Merlin spec, Merlin will be instructed to start each run using the JSON HELICS runner files.

#### Merlin Specification

##### Environment

In the Merlin spec we will instruct Merlin to execute N number of the
Monte Carlo co-simulations. The number of samples is the number
specified as the `N_SAMPLES` env variable in the env section of the
Merlin spec.

```yaml
env:
  variables:
    OUTPUT_PATH: ./UQ_EV_Study
    N_SAMPLES: 10
```

We set the output directory to `UQ_EV_Study`, this is where all the
output files will be stored. Every co-simulation run executed by
Merlin will have its own subdirectory in `./UQ_EV_Study`.

##### Merlin Step

Remember this step is for Merlin to setup all the files and data it
needs to execute its jobs. In the Monte Carlo co-simulation there is
a python script we created that will generated the HELICS runner
files that Merlin will use when it executes a specific run. The
`make_samples_merlin.py` script (located under the `simple` subfolder of
the advanced orchestration example code) will also output a csv file that
Merlin will use. The csv file contains all the names of the runner files.
Merlin will go down this list of file names and launch each co-simulation using the HELICS runner JSON file.

```yaml
merlin:
  samples:
    generate:
      cmd: |
        python3 $(SPECROOT)/simple/make_samples_merlin.py $(N_SAMPLES) $(MERLIN_INFO)
        cp $(SPECROOT)/Battery.py $(MERLIN_INFO)
        cp $(SPECROOT)/Charger.py $(MERLIN_INFO)
    file: $(MERLIN_INFO)/samples.csv
    column_labels: [FED]
```

The samples the get generated should look something like below. This
is the runner file that the HELICS runner will use to start the
co-simulation.

```json
{
  "federates": [
    {
      "directory": ".",
      "exec": "helics_broker --federates=2 --loglevel=5 --type=tcpss --port 12345",
      "host": "broker",
      "name": "broker_0",
      "loglevel": 3
    },
    {
      "directory": ".",
      "exec": "python3 Battery.py --port 12345 --seed 1",
      "host": "broker",
      "name": "Battery",
      "loglevel": 3
    },
    {
      "directory": ".",
      "exec": "python3 Charger.py --port 12345",
      "host": "broker",
      "name": "Charger",
      "loglevel": 3
    }
  ],
  "name": "Generated by make samples"
}
```

Once the samples have been created, we copy the 2 federates to the
`MERLIN_INFO` directory.

##### Study Step

We have made it to the study step. This step will execute all 10 Monte
Carlo co-simulations. There are 2 steps in the study step. The first
step is the `start_parallel_sims` step. This step will use the
HELICS runner to execute each co-simulation. The second step, `cleanup`
depends on the first step. Once `start_parallel_sims` is complete the
`cleanup` step will remove any temporary data that is no longer
needed.

```yaml
study:
  - name: start_parallel_sims
    description: Run a bunch of UQ sims in parallel
    run:
      cmd: |
        spack load helics
        helics run --path=$(MERLIN_INFO)/$(FED)
        echo "DONE"
  - name: cleanup
    description: Clean up
    run:
      cmd: rm $(MERLIN_INFO)/samples.csv
      depends: [start_parallel_sims_*]
```

Below is what the DAG of the Merlin study will look like. Each of the
Co-Sim_n bubbles represents the Monte Carlo simulation. Each co-sim
runs in parallel with each other since there is no dependency on the
output that each co-sim runs.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/UQ_DAG.png)

## [Questions and Help](../../support.md)

Do you have questions about HELICS or need help?

1. Come to [office hours](https://helics.org/HELICSOfficeHours.ics)!
2. Post on the [gitter](https://gitter.im/GMLC-TDC/HELICS)!
3. Place your question on the [github forum](https://github.com/GMLC-TDC/HELICS/discussions)!
