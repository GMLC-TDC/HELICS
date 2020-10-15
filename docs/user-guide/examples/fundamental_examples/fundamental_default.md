# Base Example Co-Simulation



```eval_rst
.. toctree::
    :hidden:
    :maxdepth: 2
    
```
The Base Example walks through a simple HELICS co-simulation 

## Components of the HELICS program

## Components of the



When we build a HELICS co-simulation, we need to 

What are we trying to simulate?  That will inform the _topology_ of the co-simulation, or how we knit together the individual simulators.

We will start by connecting two python federates. Remember that a federate[link to federate page] is the same as a simulator (with python, a complete program).

note that we're using the c interface
want people to see the same apis they would see if they're using matlab, c etc

if you are looking at these examples, and only using python, odds are you're going to like the class-based interface


basic model, from which everything else will be built

the place for newbies.
## Register Federates, Configure Federates

What are we doing ?  We are telling HELICS to create a federate from a JSON config file.

## test

## Components to a basic co-simulation

### Pre launch housekeeping

1. Set up loggers

2. Plan to terminate federates at completion

We can destroy the federates if the routine completes, but what should we do if the routine crashes prior to completion?

### Main Routine

#### Register federates