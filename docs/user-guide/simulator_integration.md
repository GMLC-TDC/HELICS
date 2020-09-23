# Integrating a Simulator with HELICS

At some point, maybe from the very beginning of your time with HELICS co-simulation, you'll have an interest or need to include a simulator in your co-simulation that isn't already integrated into HELICS. Maybe it's an existing open-source simulator, maybe it's commercial software, maybe it's a small controller simulator you'd like to test in an existing model. HELICS has been designed to make it as easy as possible to integrate a new simulator. Before writing code, though, it is important to more specifically define the task.

## Simulator Integration Clarifying Questions

1. **What is the nature of the code-base being integrated?** Is this open-source code that can be fully modified? Is it a simulator, perhaps commercial, that provides an API that will be used? How much control do you, the integrator, have in modifying the behavior of the simulator?
2. **What programming language will be used?** - HELICS has bindings for a number of languages and the one that is best to use may or may not be obvious. If your integration of the simulator will be through the API of the existing simulator, then you'll likely be writing a standalone executable that wraps that API. You may be constrained on the choice of languages based on the method of interaction with that API. If the API is accessed through a network socket then you likely have a lot of freedom in language choice. If the API is a library that you call from within wrapper, you will likely be best served using the language of that library.

   If you're writing your own simulator then you have a lot more freedom and the language you use may come down to personal preference and/or performance requirements of the federate.

   The languages currently supported by HELICS are:

   - C++
   - C
   - Python (2 and 3)
   - Java
   - MATLAB
   - Octave
   - C# (somewhat limited as of yet)
   - Julia
   - Nim

3. **What is the simulator's concept of time?** - Understanding how the simulator natively moves through time is essential when determining how time requests will need to be made. Does the simulator have a fixed time-step? Is it user-definable? Does the simulator have any concept of time or is it event-based?
4. **What is the nature of the data it will send to and receive from the rest of the federation?** Often, this answer is in large part provided by the analysis need that is motivating the integration. However, there may be other angles to consider beyond what's immediately apparent. As a stand-alone simulator, what are its inputs and outputs? What are its assumed or provided boundary conditions? Where do interdependencies exist between the simulator and other simulators within the federation? What kinds of data will it be providing to the rest of the federation?

## The Essential API Calls

With the answers to those clarifying questions in mind, let's look at how you might go about creating a HELICS federate, which is essentially the agent that enables your simulator to interface with the rest of the co-simulation. For the remainder of this section of the guide, we'll walk through the typical stages of co-simulation, providing examples of how these might be implemented using HELICS API calls. For the purposes of these examples, we will assume the use of a Python binding. If, as the simulator integrator, you have needs beyond what is discussed here you'll have to dig into the [developer documentation on the APIs](../doxygen/index.md) to get the details you need.

To begin, at the top of your Python module ([after installing the Python HELICS module](https://helics.readthedocs.io/en/latest/installation/index.html)), you'll have to import the HELICS library, which will look something like this:

```python
import helics as h
```

### Federate Information

Each federate has a core set of configuration information and metadata associated with it, which will either need to be set within your code or will be set based on defaults. When creating a new federate, only one piece of metadata is actually required, and that is the federate name, which must be unique within the federation. However, there are many other configuration options that can be set for the federate, including whether the federate can be interrupted between its native time steps, a minimum time step for its execution and the level to use when the federate logs information. Information on all of these configuration options, including default settings, can be found [here](./../configuration/FederateFlags.md). 

### Publications, Subscriptions and Endpoints

One of the first design choices you have to make is the type of federate that you will create to instantiate your simulator within the co-simulation. At this point, we will revisit the question on what kind of data you expect your the simulator to exchange with the rest of the federation. There are three kinds of federates within HELICS: [value federates](./value_federates.md), [message federates](./message_federates.md), and combination federates. 

Value federates are used to exchange values through HELICS using a publication/subscription architecture, where only a single value can be received at a given subscription at each time step. Value federates are used to represent physics-based interdependencies. An example of where the exchange of values is probably most appropriate is where the same data point is represented in two different simulators, such as the voltage at a transmission bus that corresponds to the voltage at a distribution feeder head. 

By contrast, message federates are used to exchange messages through HELICS that look and behave more like communications-based data. Examples of this might include control signals or measurement data. This is done using endpoints, rather than publications and subscriptions, and unlike in the value case, more than one message can be received at an endpoint at any given time step. A combination federate is one that handles both values and messages. More details on the differences between these federate types are provided elsewhere in this guide.

### Create the HELICS Federate

Now that you've decided what kind of federate you are going to use to instantiate your simulator within the federation, you'll need to actually create that federate in your code. There are two ways to do this: from a configuration file or programmatically, using a sequence of HELICS API calls. In most instances, using a configuration file is probably simpler and more modular. However, we will go through both options below as there may be times when creating the federate in your source code is necessary or more appropriate.

#### Using a Config File

In HELICS there is a single API call that can be used to read in all of the necessary information for creating a federate from a JSON configuration file. The JSON configuration file, as discussed earlier in this guide, contains both the federate info as well as the metadata required to define the federate's publications, subscriptions and endpoints. The API calls for creating each type of federate are given below. 

For a value federate:

```python
fed = h.helicsCreateValueFederateFromConfig('fed_config.json')
```

For a message federate:

```python
fed = h.helicsCreateMessageFederateFromConfig('fed_config.json')
```

For a combination federate:

```python
fed = h.helicsCreateCombinationFederateFromConfig('fed_config.json')
```

In all instances, this function returns the federate object `fed` and requires a path to the JSON configuration file as an input.

#### Using HELICS API Calls

Additionally, there are ways to create and configure the federate directly through HELICS API calls, which may be appropriate in some instances. First, you need to create the federate info object, which will later be used to create the federate:

```python
fi = h.helicsCreateFederateInfo()
```

Once the federate info object exists, HELICS API calls can be used to set the [configuration parameters](./../configuration/FederateFlags.md) as appropriate. For example, to set the the only_transmit_on_change flag to true, you would use the following API call:

```python
h.helicsFederateInfoSetFlagOption(fi, 6, True)
```

Once the federate info object has been created and the appropriate options have been set, the helics federate can be created by passing in a unique federate name and the federate info object into the appropriate HELICS API call. For creating a value federate, that would look like this:

```python
fed = h.helicsCreateValueFederate(federate_name, fi)
```

Once the federate is created, you now need to define all of its publications, subscriptions and endpoints. The first step is to create them by registering them with the federate with an API call that looks like this:

```python
pub = h.helicsFederateRegisterPublication(fed, key, data_type)
```

This call takes in the federate object, a string containing the publication key (which will be prepended with the federate name), and the data type of the publication. It returns the publication object. Once the publication, subscription and endpoints are registered, additional API calls can be used to set the info field in these objects and to set certain options. For example, to set the only transmit on change option for a specific publication, this API call would be used:

TODO: Update the below with the flag string one pyHELICS library is live.
```python
pub = h.helicsPublicationSetOption(pub, 454, True)
```

Once the federate is created, you also have the option to set the federate information at that point, which - while functionally identical to setting the federate info in either the federate config file or in the federate info object - provides integrators with additional flexibility, which can be useful particularly if some settings need to be changed dynamically during the cosimulation. The API calls are syntatically very similar to the API calls for setting up the federate info object, except instead they target the federate itself. For example, to revist the above example where the only_transmit_on_change on change flag is set to true in the federate info object, if operating on an existing federate, that call would be:

TODO: Update the below with the flag string one pyHELICS library is live.
```python
h.helicsFederateSetFlagOption(fi, 6, True)
```

### Error Handling

By default, HELICS will not terminate execution of every participating federate if an error occurs in one. However, in most cases, if such an error occurs, the cosimulation is no longer valid. It is therefore generally a good idea to set the following flag in your simulator federate so that its execution will be terminated if an error occurs anywhere in the cosimulation.

```python
h.helicsFederateSetFlagOption(fed, helics_flag_terminate_on_error)
```
This will prevent your federate from hanging in the event that another federate fails.


### Collecting the Publication, Subscription and Endpoint Objects

Having configured the publications, subscriptions and endpoints and registered this information with HELICS, the channels for sending and receiving this information have been created within the cosimulation framework. If you registered the publication, subscriptions and endpoints within your code (i.e., using HELICS API calls), you already have access to each respective object as it was returned when made the registration call. However, if you created your federate using a configuration file which contained all of this information, you now need to retrieve these objects from HELICS so that you can invoke them during the execution of your cosimulation. The following calls will allow you to query HELICS for the metadata associated with each publication. Similar calls can be used to get input (or subscription) and endpoint information.

```python
pub_count = h.helicsFederateGetPublicationCount(fed)
pub = h.helicsFederateGetPublicationByIndex(fed, index)
pub_key = h.helicsPublicationGetKey(pub)
```

The object returned when the helicsFederateGetPublicationByIndex() method is invoked is the handle used for retreiving other publication metadata (as in the helicsPublicationGetKey() method) and when publishing data to HELICS (as described in the execution section below). 

### Federate Execution

Once the federate has been created, all subscriptions, publications and endpoints have been registered and, all the federate information has been appropriately set, it is time to enter executing mode. This can be done with the following API call:

```python
h.helicsFederateEnterExecutingMode(fed)
```

This method call is a blocking call; your custom federate will sit there and do nothing until all other federates have also finished any set-up work and have also requested to begin execution of the co-simulation. Once this method returns, the federation is effectively at simulation time of zero.

At this point, each federate will now set through time, exchanging values with other federates in the cosimulation as appropriate. This will be implemented in a loop where each federate will go through a set of prescribed steps at each time step. At the beginning of the cosimulation, time is at the zeroth time step (t = 0). Let's assume that the cosimulation will end at a pre-determined time, t = max_time. The nature of the simulator will dictate how the time loop is handled. However, it is likely that the cosimulation loop will start with something like this:

```python
t = 0
while t < end_time:
	# cosimulate
```
Now, the federate begins to step through time. For the purposes of this example, we will assume that during every time step, the federate will first take inputs in from the rest of the cosimulation, then make internal updates and calculations and finish the time step by publishing values back to the rest of the cosimulation before requesting the next time step. 

#### Get Inputs

The federate will first listen on each of its inputs (or subscriptions) and endpoints to see whether new information has been sent from the rest of the federation. The first code sample below shows how information can be retrieved from an input (or subscriptions) through HELICS API calls by passing in the subscription object. As can be seen, HELICS has built in type conversion ([where possible](https://www.youtube.com/watch?v=mZOAn-3aATY)) and regardless of how the sender of the data has formatted it, HELICS can present it as requested by the appropriate method call.

```python
int_value = h.helicsInputGetInteger(sub)
float_value = h.helicsInputGetDouble(sub)
real_value, imag_value = h.helicsInputGetComplex(sub)
string_value = h.helicsInputGetChar(sub)
...
  
```
It may also be worth noting that it is possible on receipt to check whether an input has been updated before retrieving values. That can be done using the following call:
```python
updated = h.helicsInputIsUpdated(sid)
```
Which returns true if the value has been updated and false if it has not.

Receiving messages at an endpoint works a little bit differently than when receiving values through a subscription. Most fundamentally, there may be multiple messages queued at an endpoint while there will only ever be one value received at a subscription (the last value if more than one is sent prior to being retrieved). To receive all of the messages at an endpoint, they needed to be popped off the queue. An example of how this might be done is given below.

```python
while h.helicsEndpointPendingMessages(end) > 0:
	msg_obj = h.helicsEndpointGetMessageObject(end)
```
To get the source of each of the messages received at an endpoint, the following call can be used:
```python
msg_source = h.helicsMessageGetOriginalSource(msg_obj)


#### Internal Updates and Calcuations

At this point, your federate has received all of its input information from the other federates in the cosimulation and is now ready to run whatever updates or calculations it needs to for the current time step. 

#### Publish Outputs

Once the new inputs have been collected and all necessary calculations made, the federate can publish whatever information it needs to for the rest of the federation to use. The code sample below shows how these output values can be published out to the federation using HELICS API calls. As in when reading in new values, these output values can published as a variety of data types and HELICS can handle type conversion if one of the receivers of the value asks for it in a type different than published.

```python
h.helicsPublicationPublishInteger(pub, int_value)
h.helicsPublicationPublishDouble(pub, float_value)
h.helicsPublicationPublishComplex(pub, real_value, imag_value)
h.helicsPublicationPublishChar(pub, string_value)
...
```
For sending a message through an endpoint, that once again looks a little bit different, in this case because - unlike with a publication - a message requires a destination. If a default destination was set when the endpoint was registered (either through the config file or through calling `h.helicsEndpointSetDefaultDestination()`), then an empty string can be passed. Otherwise, the destination must be provided as shown in API call below where dest is the destination and msg is the message to be sent.
```python
h.helicsEndpointSendMessageRaw(end, dest, msg)
```


#### Request the Next Time Step

Now it is time for the federate to request the next time step and then - once granted - progress to that time step. If the uninterruptible flag has been set for the federate, than the requested time will always be what is returned. However, if the uninterruptible flag is not set, then the federate may receive a time earlier than it requests. This will happen if there have been updates to value publications to which it is subscribed or to endpoints which it has registered. 

To request time, use the following API call:

```python
t = h.helicsFederateRequestTime (fed, time_requested)
```
For certain simulators, time_requested may be the last time (t) plus a native time step. For other simulators, time_requested may always be the final time step (end_time), and it will only be granted time when there are relevant updates provided by other federates in the cosimulation. 

Like `helicsFederateEnterExecutingMode`, this method is a blocking call. Your federate will do nothing until the HELICS core has granted a time to it.

Once granted a time, the federate is woken up and can begin execution for the next time step. Remember that the granted time may or may not be the requested time as the arrival of new inputs from the federation can cause the federate to be woken up prior to the requested time. More than likely, your federate will want to check what time has been granted and may choose different paths of execution based on whether this was the requested time or not.


### Federate Finalization

Once the federate has completed its contribution to the co-simulation, it needs to close out its connection to the federation. Typically a federate knows it has reached the end of the co-simulation when it is granted `maxTime`. To leave the federation cleanly (without causing errors for itself or others in the co-simulation) the following process needs to be followed:

```python
h.helicsFederateFinalize(fed)
h.helicsFederateFree(fed)
h.helicsCloseLibrary()
```

`helicsFederateFinalize()` signals to the core and brokers that this federate is leaving the co-simulation. This process will take an indeterminate amount of time and thus it is necessary to poll the connection status to the broker. Once that connection has closed, the memory of the federate (associated with HELICS) is freed up with `helicsFederateFree()` and the processes in the HELICS library are terminated with `helicsCloseLibrary()`. At this point, the federate can safely end execution completely.

### Complete Example

In summary, your federate code - taken in its completion - will run through several stages for configuring and executing your federate. After importing the helics library, you will create the federate (either from a config file or directly using HELICS API calls), collect and organize information related to your publications, subscriptions and endpoints, and then enter the HELICS executing mode. At this point, cosimulation execution will start and your federate will begin stepping through time along with all of the other federates participating in the cosimulation. At each time step, your federate may take in information from the rest of the comsimulation through its subscriptions and/or endpoints, make internal updates and calculations and then output information back to the rest of the cosimulation through its publications and endpoints. After each time step is completed, your federate will then request the next time step from HELICS, which allows HELICS to manage synchronization across participating federates. 

The code sample below provides a notional example of this complete workflow using the component pieces provided so far in this simulator integration guide. Note that in this example, we assume a combination federate which has subscriptions, publications and endpoints, that the federate is created from a configuration JSON file, and that the uninterruptible flag is not set. For the sake of simplicity, we will also assume that all subscription and publication data are floats.

```python
# Import the python HELICS library
import helics as h

# Define the cosimulation end time
end_time = 5

# Define the native time step for your simulator. This is the resolution for which 
# the federate will step through time unless something occurs between time steps.
time_step = 1

# Create the combination federate from a JSON configuration file ("fed_config.json")
fed = h.helicsCreateCombinationFederateFromConfig('fed_config.json')

# Collect the objects for the federate's subscriptions, publications and endpoints which 
# were registered in HELICS when the federate was created from the config file.

# Get the total number of subscriptions from HELICS
sub_count = h.helicsFederateGetInputCount(fed)
# Create a list of the subscription objects to be used when querying these subscriptions
subs = []
for index in range(0, sub_count):
	subs += [h.helicsFederateGetInputByIndex(fed, index)]

# Get the total number of publications from HELICS
pub_count = h.helicsFederateGetPublicationCount(fed)
# Create a list of the publication objects to be used when publishing data
pubs = []
for index in range(0, pub_count):
	pubs += [h.helicsFederateGetPublicationByIndex(fed, index)]

# Get the total number of endpoints from HELICS
end_count = h.helicsFederateGetEndpointCount(fed)
# ends = []
# Create a list of the endpoint objects to be used when sending and receiving messages
for index in range(0, end_count):
	ends += [h.helicsFederateGetEndpointByIndex(fed, index)]
	
# Enter executing mode
h.helicsFederateEnterExecutingMode(fed)

# Start the cosimulate time loop
t = 0
time_requested = 0
while t < end_time:

	# Ingest any data provided by other federates in the cosimulation
	# Start by checking all subscriptions
	for sub in subs:
		# Check to see whether the subscription has been updated.
		if h.helicsInputIsUpdated(sub):
			float_value = h.helicsInputGetDouble(sub)
			# In your actual code, at this point, you would probably do something with 
			# this information. Note that you can also retrieve the subscription's key
			# and info by using the following calls. This might be done here or earlier 
			# when the sub object list was created. 
			key = h.helicsSubscriptionGetKey(sub)
			info = h.helicsInputGetInfo(sub)
		
		# Now check all endpoints (assuming that you expect your federate to receive 
		# messages at its endpoints)
		for end in ends:
			# As in the subscription example above, it's possible to get the meta data 
			# associated with the endpoint. That could be done here or when the endpoint
			# objects are collected during configuration earlier in the code.
			name = h.helicsEndpointGetName(end)
			info = h.helicsEndpointGetInfo(end)
			# Get the message payload for each message pending at each endpoint
			while h.helicsEndpointPendingMessages(end) > 0:
				# Get the next message object in the queue
				msg_obj = h.helicsEndpointGetMessageObject(end)
				# Get the message payload as a string
				msg_data = h.helicsMessageGetString(msg_obj)
				# Presumably, at this point in the code, you would do something with 
				# the message payload. You can also query the message object to get some 
				# additional information about it. Probably most useful is the message 
				# source:
				msg_source = h.helicsMessageGetOriginalSource(msg_obj)
				
		# At this point in execution, you have collected all of the information available 
		# from the rest of the cosimulation for this time step. Now would be the time to 
		# make any internal updates to your model and run any calculations or simulations.
		
		# Once any internal updates and calculations are complete, it is time to publish 
		# any data or send any messages necessary back to the rest of the federates in the
		# cosimulation. In your actual code, you would send meaningful information back to 
		# HELICS. Here, we will publish the number 1.23 as our values and the string "string"
		# as our messages.
		# Start by iterating through through each publication
        for pub in pubs:
            h.helicsPublicationPublishDouble(1.23)
			# As in previous cases, metadata about the publication can be retrieved. 
            key = h.helicsPublicationGetKey(pub)
			info = h.helicsPublicationGetInfo(pub)
			
		# Now loop through all endpoints. For this example, we'll assume that a default destination 
		# was provided in the config file. 
        for end in ends:
			h.helicsEndpointSendMessageRaw(eid, '', "string")
			# And once again, you can pull some metadata about the endpoint.
            name = h.helicsEndpointGetName(eid)
            info = h.helicsEndpointGetInfo(eid)
            dest = h.helicsEndpointGetDefaultDestination(end)
		
		# Now it's time to request the next time step. Notice that in this example, because the 
		# federate is interruptible, we check to see whether the current time (t) is equal to 
		# the last requested time. If it is, that means that during the last time step we received
		# the requested time and can increment by the time step. If not, that means that the federate
		# was interrupted and was granted a time before the requested time, and we will request that 
		# time again. 
		if t == time_requested:
			time_requested = t + time_step
		t = h.helicsFederateRequestTime(fed, time_requested)
		
# After the cosimulation loop ends, it's time to finalize the federate.
h.helicsFederateFinalize(fed)
h.helicsFederateFree(fed)
h.helicsCloseLibrary()
            
		
				
		
				
```
