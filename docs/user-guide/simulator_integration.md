# Integrating a Simulator into HELICS #

At some point, maybe from the very beginning of your time with HELICS co-simulation, you'll have an interest or need to include a simulator in your co-simulation that HELICS doesn't support. Maybe its an existing open-source simulator, maybe its commercial software, maybe its a small controller simulator you'd like to test in an existing model. HELICS has been designed to make it as easy as possible to integrate a new simulator. Before writing code, though, it is important to more specifically define the task.

## Simulator Integration Clarifying Questions##
  1. **What is the nature of the code-base being integrated?** Is this open-source code that can be fully modified? Is it a simulator, perhaps commercial, that provides an API that will be used?  How much control do you, the integrator, have in modifying the behavior of the simulator?
  2. **What programming language will be used?** - HELICS has bindings for a number of languages and the one that is best to use may or may not be obvious. If you're integration of the simulator will be through the API of the existing simulator, then you'll likely be writing a standalone executable that wraps that API. You may be constrained on the choice of languages based on the method of interaction with that API. If the API is accessed through a network socket then you likely have a lot of freedom in language choice. If the API is a library that you call from within wrapper, you will likely be best of using the language of that library.

  If you're writing your own simulator then you have a lot more freedom and the language you use may come down to personal preference and/or performance requirements of the federate.
  
  The languages currently supported by HELICS are: 
    - C++
    - C
    - Python (2 and 3)
    - Java
    - MATLAB
    - Octave
  3. **What is the simulators concept of time?** - Understanding how the simulator natively moves through time is essential when determining how time requests will need to be made. Does the simulator have a fixed timestep? Is it user-definable? Does the simulator have any concept of time or is it event-based?
  4. **What is the nature of the values it will send to and receive from the rest of the federation?** Depending on the nature of the simulator, this may or may not be specifically definable but a general understanding of how this simulator will be used in a co-simulation should be clear. As a stand-alone simulator, what are its inputs and outputs? What are its assumed or provided boundary conditions? What kinds of values will it be providing to the rest of the federation?

## The Essential APIs ##
With the answers to those clarifying questions in mind, let's look at the normal execution process used by a HELICS federate when co-simulating and the associated APIs for each of the languages. Many of these APIs are wrappers for one or more lower level APIs; additionally, there are many more detailed APIs that won't be discussed at all. If, as the simulator integrator, you have needs beyond what is discussed here you'll have to dig into the developer documentation on the APIs to get the details you need.

(xxxxxxx - Is this really federate creation and initialization or core configuration and initialization?)

### HELICS Core Creation ###
Given one or more existing simulators that need to be integrated, at some point in the code it will be necessary to create a federate instance of that simulator. Doing so established the message-passing and synchronization infrastructure that is required to be part of a HELICS co-simulation. 

The easiest way to do this is using a specific API that reads in a user-defined JSON file to create the federate. These are the configuration files that we have been examining in part or whole throughout the tutorial and are used in the examples. By placing all the configuration information in the JSON file, it allows maximum modularity and flexibility of the simulator being integrated. Using the JSON file allows all future users of the simulator to modify and customize the connection between that simulator and any given HELICS federation without having to modify the source code.

There are ways to programmatically ("hard-code") the configuration of the federate and for small, one-off simulators (like an EV charge controller, for example), doing so may be the fastest way to get the HELICS co-simulation up and running. Then again, how often does one-off code stay one-off....

Below is an example of a portion of the JSON configuration file that shows the particular information used in creating a federate. 

(xxxxxxx - show configuration file options used during federate creation.)

The APIs listed below read the entire JSON configuration file and pass the appropriate information to the lower-level APIs as the co-simulation is assembled.

(xxxxxxx - Provide code examples of this process taking place in all supported bindings)


### Federate Configuration ###
Assuming the simulator 

### Federate Initialization ###

helicsFederateEnterInitializationMode is a blocking call until the entire federation is ready to enter initialization.

### Federate Execution ###
Time Request

Publish values

Respond to received values

All kinds of interesting things can happen here.

### Federate Finalization ###

