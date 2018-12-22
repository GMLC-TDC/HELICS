#Message Federates #

xxxxxxx - message federates

## Message Federate Endpoints ##
As previously discussed, message federates interact with the federation by defining an "endpoint" that acts as their address to receive messages. Message federates are typically sending and receiving measurements, control signals, commands, and other discrete data with HELICS acting as a perfect communication system (infinite bandwidth, virtually no latency, guaranteed delivery). 

In fact, as you'll see in [a later section](./filters.md), it is possible to create more realistic communication-system effects natively in HELICS (as well as use a full-blown communication simulator like [ns-3](https://github.com/GMLC-TDC/ns-3-dev-git) to do the same). This is relevant now, though, because it influences how the endpoints are created and, as a consequence, how the simulator handles messages. You could, for example, have a system with three federates that are in communication: a remote voltage sensor, a voltage controller, and a voltage regulation actuator (we'll pretend for the case of this example that the last two are physically separated though they often aren't). In this case, you could imagine that the voltage sensor only sends messages to the voltage controller and the voltage controller only sends messages to the voltage regulation actuator. That is, those two paths between the three entities are distinct, have no interaction, and have unique properties (though they may not be modeled). Given this, referring to the figure below, the voltage sensor could have one endpoint ("Endpoint 1") to send the voltage signal, the voltage regulator could receive the measurement at one endpoint ("Endpoint 2") and send the control signal on another ("Endpoint 3"), and the voltage regulation actuator would receive the control signal on its own endpoint ("Endpoint 4").

![voltage regulation message federates](../img/voltage_reg_message_federate.pdf)

The federate code handling these messages can be relatively simple because the data coming in or going out of each endpoint is unique. The voltage controller always receives (and only receives) the voltage measurement at one endpoint and similarly only sends the control signal on another.

Consider a counter-example: automated meter-reading (AMI) using a wireless network that connects all meters in a distribution system to a data-aggregator in the substation (where, presumably, the data travels over a dedicated wired connection to a control room). All meters will have a single endpoint over which they will send their data but what about the receiver? The co-simulation could be designed with the data-aggregator having a unique endpoint for each meter but this implies come kind of dedicated communication channel for each meter; this is not the case with wireless communication. Instead, it is probably better to create a single endpoint representing the wireless connection the data aggregator has with the AMI network. In this case, messages from any of the meters in the system will be flowing through the same endpoint and to differentiate the messages from each other, the federate will have to be written to examine the metadata with the message to determine its original source.

![ami message federates](../img/ami_message_federate.pdf)


## Message Federate Configuration in JSON ##
Once the message topology considering endpoints has been determined, the definitions of these endpoints in the JSON file is straight-forward. Here's what it could look like for the voltage regulator example from above.

(xxxxxxx - JSON config for voltage regulator example)



## Example 1b - Distribution system EV charge controller ##
To demonstrate how a message federate interacts with the federation, let's take the previous example and add two things to it: electric vehicle (EV) loads in the distribution system, and a centralized EV charge control manager.

Keeping in mind that this a model for demonstration purposes (which is to say, don't take this too seriously), let's make the following assumptions to simplify the behavior of the EV charge controller:

  * All EVs have infinite battery capacity
  * All EVs will be at home all day, desiring to charge all day if they can. 
  * All EVs charge at the same power level.
  * The EV charge controller has direct control over the charging of all EVs in the distribution system. It can tell them when to turn off and on at will.
  * The EV charge controller has the responsibility to limit the total load of the distribution system to a specified level to prevent overloading on the substation transformer.
  * The EV will turn off some EV charging when the total distribution load exceeds the transformer limit by a certain percentage and will turn some EVs back on when below the limit by a certain percentage.
  * Nothing is fair about how the charge controller chooses which EVs to charge and which to disconnect.  

The message topology (including the endpoints) and the not very interesting broker topology are shown below.

![Ex. 1b message topology](../img/ex1b_message_topology.pdf)

![Ex. 1b message topology](../img/ex1b_broker_topology.pdf)


Taking these assumptions and specifications, it is not too difficult to write a simple charge controller as a Python script. And just by opening the JSON configuration file we can learn important details about how the controller works.

(xxxxxxx - insert EV charge controller JSON config)

We can see that the charge controller subscribes to the total substation load and sends out power charge commands to each of the individual EVs.

Running the example (located at xxxxxxxx) and looking at the output file (xxxxxxx) which the charge controller federate wrote out, we can see the total number of EVs charging at any point in time as well as the load on the substation throughout the day.

(xxxxxxx - insert graph of number of EVs and substation load vs time)

As you can see in the data, every time the load on the system exceeded the transformer rating by xxxxxxxx% (xxxxxxxx - specific value), the EV charge controller  disconnected the appropriate number of EVs to drop the load below the limit. Conversely, as the load dropped below the rated limit, the EV charge controller was able to connect more EVs for charging. 

(xxxxxxxx - Look at impacts of EV charge controller on transmission system; this is why we do co-simulation )

