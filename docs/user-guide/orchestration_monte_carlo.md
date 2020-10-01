
# Monte Carlo Simulation using Merlin

This tutorial will walk through how to setup a HELICS Monte Carlo simulation
using Merlin. We assume that you have already completed the
[**orchestration tutorial**](orchestration.md) and have some
familiarity with how Merlin works.

We will walk through how to user Merlin to setup and run a Monte Carlo simulation using HELICS. Code for the Monte Carlo simulation and the
full Merlin spec and be found in the [**User Guide Examples**](https://github.com/GMLC-TDC/HELICS/tree/v3userguide/examples/user_guide_examples/) under [**Example 3**](https://github.com/GMLC-TDC/HELICS/tree/v3userguide/examples/user_guide_examples/Example_3).

# TEST!!!!

this is a test to see if U+03BB works or λ or $\lambda$ or _λ_ or $$\lambda$$ or $\lambda $ 

# Monte Carlo Cosimulation

A Monte Carlo simulation allows the practitioner to sample random numbers repeatedly from a predefined distribution to explore and quantify uncertainty in their analysis. Additional detail about Monte Carlo methods can be found on [Wikipedia](https://en.wikipedia.org/wiki/Monte_Carlo_method) and [MIT Open Courses](https://www.youtube.com/watch?v=OgO1gpXSUzU).

In a Monte Carlo Cosimulation, a probability distribution of possible values can be   used in the place of any static value in any of the simulators. For example, a cosimulation may consist of a simulator (federate) which measures the voltage across a distribution transformer. We can quantify measurement error by replacing the deterministic (static) value of the measurement with a random value from a uniform distribution. Probabilistic distributions are typically described with the following notation:

$$ M \sim U(a,b) $$

Where $M$ is the measured voltage, $a$ is the lower bound for possible values, and $b$ is the upper bound for possible values. This is read as, "$M$
 is distributed uniformly with bounds $a$ and $b$." 
 
 ![](../img/uniform_dist.png)

The uniform distribution is among the most simple of probability distributions. Additional resources on probability and statistics are plentiful; [Statistical Rethinking](https://xcelab.net/rm/statistical-rethinking/) is highly recommended.

## Monte Carlo Cosim Example: EV Garage Charging

The example cosimulation to demonstrate Monte Carlo distribution sampling is that of an electric vehicle (EV) charging hub. Imagine a parking garage that only serves EVs, has a static number of charging ports, and always has an EV connected to a charging port. An electrical engineer planning to upgrade the distribution transformer prior to building the garage may ask the question: What is the likely power draw that EVs will demand?

### Probability Distributions

*Likely* is synonymous for *probability*. As we are interested in a probability, we cannot rely on a deterministic framework for modeling the power draw from EVs. I.e., we cannot assume that we know a priori the exact demand for Level 1, Level 2, and Level 3 chargers in the garage. A deterministic assumption would be equivalent to stating, e.g., that 30% of customers will need Level 1 charge ports, 50% will need Level 2, and 20% will need Level 3. What if, instead of static proportions, we assign a distribution to the need for each level of charge port. The number of each level port is discrete (there can't be 0.23 charge ports), and we want the number to be positive (no negative charge ports), so we will use the [Poisson distribution](https://en.wikipedia.org/wiki/Poisson_distribution). The Poisson distribution is a function of the anticipated average of the value $\lambda$ and the number of samples $k$. Then we can write the distribution for the number of chargers in each level, $L$, as

 $$ L \sim P(k,\lambda) $$

Let's extend our original assumption that the distribution of chargers is static to Poisson distributed, and let's assume that there are 100 total charging ports:

 $$ L1 \sim P(100,0.3) $$
 $$ L2 \sim P(100,0.5) $$
 $$ L3 \sim P(100,0.2) $$
 
  ![](../img/EVPoisson.png)
 
 What if we weren't entirely certain that the average values for $L1, L2, L3$ are $0.3, 0.5, 0.2$, we can also sample the averages from a normal distribution centered on these values with reasonable standard deviations. We can say that:
 
 $$ \lambda \sim N(\mu,\sigma) $$
 
 Which means that the input to $L$ is distributed normally with average $\mu$ and standard deviation $\sigma$.
 
 Our final distribution for modeling the anticipated need for each level of charging port in our $k = 100$ EV garage can be written as:
 
 $$ L \sim P(k,\lambda) $$
 $$ \lambda \sim N(\mu,\sigma)  $$
 
<center>

|			  | $L1$   |      $L2$      |  $L3$ |
|----------|:----------|:-------------:|------:|
| $\mu$ |  0.3|  0.5 | 0.2|
| $\sigma$ |  $\sim N(1,3)$ |    $\sim N(1,2)$   |   $\sim N(0.05,0.25)$ |

</center>

  ![](../img/EVfulldist.png)
  
Notice that the individual overplotted distributions in the two histograms above are different -- there is more flexibility encoded into the second. The distributions in the second plot describe the following assumptions about the anticipated need for Level 1, 2, and 3 chargers:

1. The number of charging ports is discrete and positive (Poisson).
2. The variance in the number of ports should be normally distributed.
3. The average percentage of Level 1, 2, and 3 chargers is 30%, 50%, and 20%, respectively.
4. The variance around the average is uniformly distributed.
5. The variance is relatively very broad for Level 1, broad for Level 2, and very constrained for Level 3. This means we are very confident in our guess of the average percentage for Level 3 chargers, less confident for Level 2, and much less confident for Level 3.

We have described the individual distributions for each level of charging port. What we don't know, prior to running the Monte Carlo simulation, is how these distributions will jointly impact the research question.

### Research Question Configuration

We want to address the research question: What is the likely power draw that EVs will demand? 

In [**Example 3**](https://github.com/GMLC-TDC/HELICS/tree/v3userguide/examples/user_guide_examples/Example_3), there are two python [**federates**](federates.md) -- one to simulate any number of EVs, and another to simulate the parking garage charge controller, which will dictate whether an EV can continue to charge. At the beginning of the cosimulation, the distributions defined above will be sampled $N$ times within the EV federate, where $N =$ the number of parking spots/charging ports in the garage ($N = 100$ in Example 3). The output of the initial sampling is the number of requested Level 1, 2, and 3 charging ports. The states of charge (SOC) for the batteries on board are initialized to a uniform random number between 0.05 and 0.5, and these SOC are sent to the EV Controller federate. If the SOC of an EV battery is less than 0.9, the EV Controller federate tells the EV battery in the EV federate to continue charging. Otherwise, the EV Controller discharges the EV battery, and instructs the EV federate to sample a new EV battery from the distributions (one sample).

After the two federates pass information between each other -- EV federate sends SOC, EV Controller federate instructs whether to keep charging or resample the distributions -- the EV federate calculates the total power demanded in the last time interval.

### Cosimulation Reproduction

As best practice, we recommend setting a seed for a single cosimulation. Management of multiple iterations of the cosimulation can be done by setting the seed as a function of the number of brokers, where there will be one broker for each iteration. In Python 3.X:

```
import argparse

parser = argparse.ArgumentParser(description='EV simulator')
parser.add_argument('--seed', type=int, default=0,
                help='The seed that will be used for our random distribution')
parser.add_argument('--port', type=int, default=-1,
                help='port of the HELICS broker')


args = parser.parse_args()
np.random.seed(args.seed)
```

The cosimulation in [**Example 3**](https://github.com/GMLC-TDC/HELICS/tree/v3userguide/examples/user_guide_examples/Example_3) was done with 10 iterations and seeds ranging from 0 to 9. The output for the Monte Carlo cosimulations with these seeds can be found with the source code. 

The result of running the cosimulation 10 times tells us that we can anticipate needed 

 
# Merlin spec for Cosimulation
In this specification we will be using the
[helics_cli](https://github.com/GMLC-TDC/helics-cli) to execute each
cosimulation run since this is a Monte Carlo simulation. This means
that helics\_cli will be executed multiple times with different
helics_cli runner files. 

## Helics_cli in Merlin

Since we are using the helics\_cli to manage and execute all the
federates, we need to create these runner files for helics_cli.
There is a provided python script called `make_samples.py` that will
generate the runner file and a csv file that will be used in the
study step.

An example of how the helics_cli runner file looks like is shown
below.

```
Example of helics_cli runner for UQ EV example
```

As you can see from the example there are 3 federates 1 for the
EVMsgFed.py, 1 for the EVControllerMsgFed.py and 1 for the HELICS
broker. Helics\_cli will start each of these federates. In the Merlin
spec, Merlin will be instructed to execute the helics\_cli with all the
generated helics\_cli runner files.

## Merlin Specification

### Environment

In the Merlin spec we will instruct Merlin to execute N number of the
Monte Carlo co-simulations. The number of samples is the number
specified as the `N_SAMPLES` env variable in the env section of the
merlin spec.

```
env:
  variables:
    OUTPUT_PATH: ./UQ_EV_Study
    N_SAMPLES: 10	
```

We set the ouput directory to UQ_EV_Study, this is where all the
output files will be stored. Every co-simulation run executed by
merlin will have it's own subdirectory in `./UQ_EV_Study`.

### Merlin Step

Remeber this step is for Merlin to setup all the files and data it
needs to execute it's jobs. In the Monte Carlo co-simulation there is
a python script we created that will generated the helics_cli runner
files that Merlin will use when it executes the helics_cli. The
`make_samples.py` script will also output a csv file that Merlin will
use. The csv file contains all the names of the runner files. Merlin
will go down this list of file names and execute the helics_cli for
each file name.

```
merlin:
  samples:
    generate:
      cmd: |
        python3 $(SPECROOT)/make_samples.py $(N_SAMPLES) $(MERLIN_INFO)
        cp $(SPECROOT)/EVMsgFed.py $(MERLIN_INFO)
        cp $(SPECROOT)/EVControllerMsgFed.py $(MERLIN_INFO)
    file: $(MERLIN_INFO)/samples.csv
    column_labels: [FED]

```

The samples the get generated should look something like below. This
is the runner file that helics_cli will use to start the
co-simulation.

```
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
            "exec": "python3 EVMsgFed.py --port 12345 --seed 1",
            "host": "broker",
            "name": "EVMsgFed_0",
            "loglevel": 3
        },
        {
            "directory": ".",
            "exec": "python3 EVControllerMsgFed.py --port 12345",
            "host": "broker",
            "name": "EVController_0",
            "loglevel": 3
        }
    ],
    "name": "Generated by make samples"
}
```

Once the samples have been created, we copy the 2 federates to the
`MERLIN_INFO` directory.

### Study Step

We have made it to the study step. This step will execute all 10 Monte
Carlo co-simulations. There are 2 steps in the study step. The first
step is the `start_parallel_sims` step. This step will use the
helics_cli to execute each co-simulation. The second step, `cleanup`
depends on the first step. Once `start_parallel_sims` is complete the
`cleanup` step will remove any temporary data that is no longer
needed.

```

study:
    - name: start_parallel_sims
      description: Run a bunch of UQ sims in parallel
      run:
        cmd: |
          spack load helics
          /home/yee29/projects/helics/helics-cli/bin/helics run --path=$(MERLIN_INFO)/$(FED)
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

![](../img/UQ_DAG.png)



