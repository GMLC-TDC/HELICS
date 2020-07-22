# Orchestration

The goal of this guide is to show and guide you how you might handle
co-simulation orchestration. We will walk through using a specific
tool ([merlin](https://github.com/LLNL/merlin)) that has tested with
HELICS co-simulations. This is not the only tool that exist that has
this capability and is not a requirement if all you want to achieve
co-simulation orchestration. One advantage that Merlin has, is its
ability to interface with HPC systems that have SLURM or Flux as their
resource managers.

# Definition of "Orchestration" in HELICS

We will define the term "orchestration" within HELICS as a method to
manage and execute a single co-simulation cycle or multiple
co-simulation cycles. This includes managing output from 1
co-simulation so that it can be used as input to subsequent
co-simulation runs if needed.

## What you will need

First you will need to build and install
[merlin](https://github.com/LLNL/merlin). This guide will walk through
a suggested co-simulation spec using merlin to launch a HELICS
co-simulation.

### Merlin

A merlin specification has 2 main parts that control how a
co-simulation may run. Some steps are not required so use the onese
that are most appropriate for your co-simulation. Below we describe
how you might use each step. We will be using the pi-exchange python
example that can be found
[here](https://github.com/GMLC-TDC/HELICS-Examples/tree/master/python/pi-exchange). The
goal will be to have merlin launch multiple pi-senders and
pi-recievers.

#### Merlin Spec variables and description
In addition to the main co-simulation launching steps, merlin also has
description and and environment block. The description block,
describes the name and a short description about the study.
```
description:
    name: Test helics 
    description: Juggle helics data
```

The env block describes the environment that the study will execute
in. This is a place where you can set environment variables to control
the number of federates you may need in your co-simulation.

```
env:
  variables:
    OUTPUT_PATH: ./helics_juggle_output
    N_SAMPLES: 8
```



#### Merlin Step

The merlin step can simply be summarized as the input data generation
step. This step describes how to create the initial inputs for the
co-simulation so that subsquent steps can use this input to start the
co-simulation. Below is how we might decribe the merlin step for our
pi-exchange study.

```
merlin:
  samples:
    generate:
      cmd: |
        python3 $(SPECROOT)/make_samples.py $(N_SAMPLES) $(MERLIN_INFO)
        cp $(SPECROOT)/pireceiver.py $(MERLIN_INFO)
        cp $(SPECROOT)/pisender.py $(MERLIN_INFO)
    file: samples.csv
    column_labels: [FED]

```
