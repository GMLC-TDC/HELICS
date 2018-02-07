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
[hello\_world\_sender.c](https://github.com/GMLC-TDC/HELICS-src/blob/master/examples/CInterface/hello_world_sender.c)
and paste it into the file.

Next, create a new source file and call it `hello_world_receiver.c`.
Copy the contents from
[hello\_world\_receiver.c](https://github.com/GMLC-TDC/HELICS-src/blob/master/examples/CInterface/hello_world_receiver.c)
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

Be sure to use the same compiler you used to build the HELICS library.
You may need to include additional include paths and library paths in
the above command.

**Running a federation**

Linux and Mac:

Next, open three terminals. In the first terminal, run the following
command.

```bash
$ ./helics_broker 2
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

**Anatomy of a HELICS federation**

Now, let’s go over what just happened in the `hello_world_sender.c` part
of the “Hello, World” program in detail.

The following block creates a ValueFederate. We will discuss what
`FederateInfo` is and what a `ValueFederate` is, along with other types
of Federates in more detail in a later chapter.

```c
fedinfo = helicsFederateInfoCreate();
helicsFederateInfoSetFederateName(fedinfo, "Test sender Federate");
helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq");
helicsFederateInfoSetCoreInitString(fedinfo, "--federates=1");
vfed = helicsCreateValueFederate(fedinfo);
```

The following registers a global publication.

```c
pub = helicsFederateRegisterGlobalPublication(vfed,"testA","string","");
```

The following ensures that the federation has entered execution mode.

```c
helicsFederateEnterInitializationMode(vfed);
helicsFederateEnterExecutionMode(vfed);
```

These functions publish a String and make a RequestTime function call.

```c
helicsPublicationPublishString(pub, "Hello, World");
helicsFederateRequestTime(vfed,currenttime, &currenttime);
```

And these functions finally frees the Federate.

```c
helicsFederateFinalize(vfed);
helicsFederateFree(vfed);
helicsCloseLibrary();
```

You can see that the `hello_world_receiver.c` is also very similar, but
uses a Subscription instead. A snippet of the code is shown below.

```c
fedinfo = helicsFederateInfoCreate ();
helicsFederateInfoSetFederateName (fedinfo, "TestB Federate");
helicsFederateInfoSetCoreTypeFromString (fedinfo, "zmq");
helicsFederateInfoSetCoreInitString (fedinfo, fedinitstring);

vfed = helicsCreateValueFederate (fedinfo);
sub = helicsFederateRegisterSubscription (vfed, "testA", "string", "");

helicsFederateEnterInitializationMode (vfed);
helicsFederateEnterExecutionMode (vfed);

helicsFederateRequestTime (vfed, currenttime, &currenttime);

isupdated = helicsSubscriptionIsUpdated (sub);
helicsSubscriptionGetString (sub, value, 128);
printf("%s\n", value);

helicsFederateFinalize (vfed);
helicsFederateFree (vfed);
helicsCloseLibrary ();
```
