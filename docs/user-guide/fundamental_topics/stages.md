# Co-simulation Stages

Helics has several stages to the co-simulation. Creation, initialization, execution, and final state. The helicsFederateEnterExecutingMode is the transition between initialization and execution. In the init mode values can be exchanged prior to time beginning. Normally values published in init mode are available at time 0, but if the iteration is used they can be available inside the initialization mode. There are 3 available options for the iterate parameter.

`no_iteration` -- don't iterate

`force_iteration` -- guaranteed iteration, stay in iteration mode

`iterate_if_needed` -- If there is data available from other federates Helics will iterate, if no additional data is available it will move to execution mode and have granted time=0.

ou could manage the timing of the federation to use the `initialize` mode in HELICS. This would allow you to communicate initial conditions (`t=0`) information prior to the start of execution mode. It would also allow you to publish out the `t=1` information so that all federates would have access to it once they were granted `t=1`. Look into `helicsFederateEnterInitializingMode()`.

## Creation

### Registration 

### Collecting the Interface Objects

Having configured the publications, subscriptions and endpoints and registered this information with HELICS, the channels for sending and receiving this information have been created within the cosimulation framework. If you registered the publication, subscriptions and endpoints within your code (i.e., using HELICS API calls), you already have access to each respective object as it was returned when made the registration call. However, if you created your federate using a configuration file which contained all of this information, you now need to retrieve these objects from HELICS so that you can invoke them during the execution of your cosimulation. The following calls will allow you to query HELICS for the metadata associated with each publication. Similar calls can be used to get input (or subscription) and endpoint information.

```python
pub_count = h.helicsFederateGetPublicationCount(fed)
pub = h.helicsFederateGetPublicationByIndex(fed, index)
pub_key = h.helicsPublicationGetKey(pub)
```

The object returned when the helicsFederateGetPublicationByIndex() method is invoked is the handle used for retreiving other publication metadata (as in the helicsPublicationGetKey() method) and when publishing data to HELICS (as described in the execution section below). 

## Initialization

## Execution

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
```

#### Internal Updates and Calculations

At this point, your federate has received all of its input information from the other federates in the co-simulation and is now ready to run whatever updates or calculations it needs to for the current time step. 

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



## Final State


Once the federate has completed its contribution to the co-simulation, it needs to close out its connection to the federation. Typically a federate knows it has reached the end of the co-simulation when it is granted `maxTime`. To leave the federation cleanly (without causing errors for itself or others in the co-simulation) the following process needs to be followed:

```python
h.helicsFederateFinalize(fed)
h.helicsFederateFree(fed)
h.helicsCloseLibrary()
```

`helicsFederateFinalize()` signals to the core and brokers that this federate is leaving the co-simulation. This process will take an indeterminate amount of time and thus it is necessary to poll the connection status to the broker. Once that connection has closed, the memory of the federate (associated with HELICS) is freed up with `helicsFederateFree()` and the processes in the HELICS library are terminated with `helicsCloseLibrary()`. At this point, the federate can safely end execution completely.
