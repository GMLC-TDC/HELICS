# Hello World

Now that you have HELICS installed, you are ready to create your first
HELICS federation. Let's create a simple `Hello, World` example with 2
federates.

<div class="admonition note">

Note: This tutorial assumes basic familiarity with the command line. The
HELICS co-simulation framework itself makes no specific demands about
your editing, tooling, or where your code lives. Feel free to use
whatever editor or IDE you are comfortable with.

</div>

**Create a federations directory**

Linux and Mac:

```bash
$ mkdir -p ~/federations/hello_world
$ cd ~/federations/hello_world
```

Windows CMD:

```bash
> mkdir %USERPROFILE%\federations
> cd %USERPROFILE%\federations
> mkdir hello_world
> cd hello_world
```

**Writing your first federation**

Next, make a new source file and call it `hello_world_sender.c`. Copy
the contents from
[hello_world_sender.c](https://github.com/GMLC-TDC/HELICS-Examples/blob/72c9d38e/c/hello_world/hello_world_sender.c)
and paste it into the file.

Next, create a new source file and call it `hello_world_receiver.c`.
Copy the contents from
[hello_world_receiver.c](https://github.com/GMLC-TDC/HELICS-Examples/blob/72c9d38e/c/hello_world/hello_world_receiver.c)
and paste it into the file.

We will go through in more detail the contents of these files. For now,
save the files and open two terminals.

**Compiling the federates**

To compile the federates, you can use the following commands.

Linux and Mac:

```bash
$ cc hello_world_sender.c -o ./hello_world_sender -lhelicsSharedLib
$ cc hello_world_receiver.c -o ./hello_world_receiver -lhelicsSharedLib
```

You may need to include additional include paths and library paths in
the above command.

**Running a federation**

Linux and Mac:

Next, open three terminals. In the first terminal, run the following
command.

```bash
$ ./helics_broker -f2
```

In the second terminal, run the following command.

```bash
$ ./hello_world_sender
```

In a third terminal, run the following command.

```bash
$ ./hello_world_receiver
```

You should see `Hello, World` printed out in the terminal where you ran
the `hello_world_receiver`.

_For a guide to run this example in Visual Studio go to this link:
[hello-world-VS](./hello-world-VS.md)._

**Anatomy of a HELICS federation**

Now, let’s go over what just happened in the `hello_world_sender.c` part
of the “Hello, World” program in detail.

The following block creates a ValueFederate. We will discuss what
`FederateInfo` is and what a `ValueFederate` is, along with other types
of Federates in more detail in other documents.

```c
fedinfo = helicsCreateFederateInfo ();
helicsFederateInfoSetCoreTypeFromString (fedinfo,"zmq",&err);
helicsFederateInfoSetCoreInitString (fedinfo,fedinitstring,&err);
helicsFederateInfoSetTimeProperty (fedinfo,helicsGetPropertyIndex("period"), 1.0,&err);
vfed = helicsCreateValueFederate ("hello_world_sender",fedinfo,&err);
```

The following registers a global publication.

```c
pub = helicsFederateRegisterGlobalPublication (vfed, "hello", helics_data_type_string, "",&err);
```

The following ensures that the federation has entered execution mode.
If `helicsFederateEnterInitializingMode` is not included the call to
`helicsFederateEnterExecutingMode` will automatically make the call in the background.

```c
helicsFederateEnterInitializingMode (vfed,&err);
helicsFederateEnterExecutingMode (vfed,&err);
```

These functions publish a String and make a RequestTime function call to
advance time in the simulation.

```c
helicsPublicationPublishString(pub, "Hello, World",&err);
currenttime=helicsFederateRequestTime(vfed, 1.0, &err);
```

And finally, these functions free the Federate and close the HELICS library.

```c
helicsFederateFinalize (vfed,&err);
helicsFederateFree (vfed);
helicsCloseLibrary ();
```

You can see that the `hello_world_receiver.c` is also very similar, but
uses a Subscription instead. A snippet of the code is shown below.

```c
fedinfo = helicsCreateFederateInfo ();
helicsFederateInfoSetCoreTypeFromString (fedinfo, "zmq",&err);
helicsFederateInfoSetCoreInitString (fedinfo, fedinitstring,&err);
helicsFederateInfoSetTimeProperty (fedinfo,helics_property_time_period, 1.0,&err);

vfed = helicsCreateValueFederate ("hello_world_receiver",fedinfo,&err);
sub = helicsFederateRegisterSubscription (vfed, "hello",NULL,&err);

helicsFederateEnterInitializingMode (vfed,&err);
helicsFederateEnterExecutingMode (vfed,&err);

/** request that helics grant the federate a time of 1.0
    the new time will be returned in currentime*/
currenttime=helicsFederateRequestTime (vfed, 1.0,&err);

isUpdated = helicsInputIsUpdated (sub);
helicsInputGetString (sub, value, 128,&actualLen,&err)
printf("%s\n", value);

helicsFederateFinalize (vfed,&err);
helicsFederateFree (vfed);
helicsCloseLibrary ();
```

**_A note on the `&err` term_**
Many functions in the C API take a pointer to a helics_error structure. This can be created by a call to `helicsErrorInitialize` and can be reset by `helicsErrorClear(helics_error *err)`. If an error occurs during the execution of a function or some inputs were invalid an error code in the helics_error structure will be set and a message included. For all functions if an error structure that already has an error in place is passed as an argument the function short circuits and does nothing. So checks can be done after a sequence of calls if desired with no worry about side effects. In the C++98 API an error triggers an exception, and in the base C++ API these originate as exceptions.
