# Python Example

In the previous section, we covered the basics of a HELICS federate and how you can run multiple federates together to form a federation.
In this section we will look at how to create a federation in Python.
We will create a simple `pi-exchange` federation in Python with 2 federates.

**HELICS Python Setup**

Before we run the Python `pi-exchange` federation, it is necessary to ensure that we have Python installed and that we have the HELICS Python built successfully and correctly on the machine.

We recommend using Anaconda3/Miniconda3, although this should work with most versions of Python.
To make an interface that is similar across different languages, we use SWIG to generate the Python bindings to the `helicsSharedLib` shared library.
SWIG claims to be compatible with most Python versions, dating back to Python 2.0. And recommends that for the best results, one should consider using Python 2.3 or newer.

Installation of HELICS on a Mac can be done using `brew` which comes with the ability to build the Python extension as well.
HELICS can also be built from source and linked with the required Python version.
See the page on the Installation instructions for more information regarding this.

**Create a federations directory**

Linux and Mac:

```bash
$ mkdir -p ~/federations/pi-exchange
$ cd ~/federations/pi-exchange
```

Windows CMD:

```bash
> mkdir %USERPROFILE%\federations
> cd %USERPROFILE%\federations
> mkdir pi-exchange
> cd pi-exchange
```

**Writing the Python federation**

Next, make a new source file and call it `pisender.py`. Copy
the contents from
[pisender.py](https://github.com/GMLC-TDC/HELICS-src/blob/develop/examples/python/pi-exchange/pisender.py)
and paste it into the file.

Next, create a new source file and call it `hello_world_receiver.c`.
Copy the contents from
[pireceiver.py](https://github.com/GMLC-TDC/HELICS-src/blob/develop/examples/python/pi-exchange/pireceiver.py)
and paste it into the file.

Save the files.

**Running a federation**

Linux and Mac:

Next, open two terminals. In the first terminal, run the following command.

```bash
$ python pisender.py
```

In a second terminal, run the following command.

```bash
$ python pireceiver.py
```

