# HELICS Quick Start

If you just want to get a HELICS co-simulation running on your local machine and to see how it works for yourself, this is the place to start. This Quick Start Guide will get HELICS installed along with the Python interface along with the first fundamental example. You'll then be able to run it, see your results, and take a look at the code. And, of course, at the end of all of that you're puzzled as to what you actually did, we have a [whole Users's Guide](../user-guide/index.md) to get you up to speed.

The commands below are are terminal/command-line tools available on Windows, Linux, and macOS.

## Install HELICS and the Python Language Binding

`pip install 'helics[cli]'`

The HELICS User Guide predominantly uses Python as, in our experience, Python is the _lingua franca_ of the application-oriented (vs. computer science) computing world. The above command installs the Python language bindings for HELICS (allowing you to add `import helics` to any Python script) as well as a HELICS library.

## Confirm installation

`helics --version` should return something reasonable-looking, namely a version number followed by unique identifier for the release.

## Clone in the HELICS Examples Repository

`git clone https://github.com/GMLC-TDC/HELICS-Examples.git`

OR

[Download a copy of the repository](https://github.com/GMLC-TDC/HELICS-Examples/archive/refs/heads/main.zip) (if you're not familiar with Git.)

The HELICS Examples repository contains all the examples for the User Guide (and other example content as well). Cloning or downloading this repository will give you a local copy of all those examples.

## Navigate to the "Fundamental Default" example

We'll be running the first example in the User Guide. From the top level of the HELICS Examples repository follow this path:

`user_guide_examples/fundamental/fundamental_default`

## Run the Fundamental Default Example

`helics run --path=fundamental_default_runner.json`

The `helics run` command provides an easy way to launch a co-simulation based on the contents of the runner file ("fundamental_default_runner.json" in this case). In this case, the runner launches two Python federates created for this example, "Battery.py" (which models five EV batteries) and "Charger.py" (which models five EV chargers). You should a few graphs that look like this:

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultbattery.png)
![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/fundamental_default_resultcharger.png)

## Next Steps

- Look through the code of ["Battery.py"](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/fundamental/fundamental_default/Battery.py) and/or ["Charger.py"](https://github.com/GMLC-TDC/HELICS-Examples/blob/main/user_guide_examples/fundamental/fundamental_default/Charger.py) to see how they work.
- If you're a little confused, look at the [documentation on the Fundamental Default example](../user-guide/examples/fundamental/fundamental_default.md)
- If you're still a little confused, start from [the beginning of the User Guide](../user-guide/index.md) to better understand HELICS principles and concepts.
- If this is all making sense now, [try running another example](../user-guide/examples/supported_language_examples_index.md) to better understand some of the other features of HELICS.
