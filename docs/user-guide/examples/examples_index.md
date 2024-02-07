# Examples

The examples provided in the user guide begin with a simple co-simulation that you should be able to execute with only python and HELICS installed. If you have not installed HELICS yet, navigate to the [**installation page**](../installation/index.md).

<span style="font-size:larger;">**What are we modeling?**</span>

The model for the examples is a co-simulation of a number of electric vehicle (EV) batteries and a charging port. A researcher may pose the question, "What is the state of charge of batteries on board EVs as they charge connected to a charging port?"

This can be addressed with a simple two-federate co-simulation, as demonstrated in the Fundamental Examples, or with a more complicated multi-federate co-simulation modeled in the Advanced Examples. In each learning path, modules are provided to the user to demonstrate a skill. The Advanced Examples build on the basics to make the co-simulation better emulate reality.

<span style="font-size:larger;">**Learning Tracks**</span>

There are two learning tracks available to those hoping to improve their HELICS skills. The Fundamental Examples are designed for users with no experience with HELICS or co-simulation. The Advanced Examples are geared towards users who are familiar with HELICS and feel confident in their abilities to build a simple co-simuluation. The Advanced Examples harness the full suite of HELICS capabilities, whereas the Fundamental Examples teach the user the basics.

These two learning tracks each start with a "base" model, which should also be considered the recommended default settings. Examples beyond the base model within a track are modular, not sequential, allowing the user to self-guide towards concepts in which they want to gain skill.

<span style="font-size:larger;">**A Word on HELICS CLI**</span>

All (or almost all) of the HELICS User Guide examples utilize a common tool and command for launching the co-simulation:

`helics run --path=runner.json`

This utilizes a tool we call `helics_cli` that is (optionally) [installed with the Python language binding](../installation/index). Not only does HELICS CLI take care of launching the co-simulation, it also manages the logging and error/warning messages that are often printed to console when running code in a stand-alone manner. Generally, using HELICS CLI is the recommended way to run a federation but if there are particular needs you have that HELICS CLI can't meet, all federations can simply be run by launching each individual federate sequentially. The contents of the HELICS CLI runner.json shows the commands it uses to launch each federate and those can simply be copied and pasted into a set of command-line prompts manually launch the co-simulation.

```{eval-rst}
.. toctree::
    :maxdepth: 1

    fundamental_examples/fundamental_examples_index
    advanced_examples/advanced_examples_index
    supported_languages_examples/supported_languages_examples_index
    misc_examples/misc_examples_index
```
