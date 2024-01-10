# Example 1b: GridLAB-D as a Message Federates

As previously discussed in the [federate introduction](../../../fundamental_topics/federates.md), message federates are used to create HELICS messages that model information transfers (versus physical values) moving between federates. Measurement and control signals are typical applications for these types of federates.

Unlike HELICS values which are persistent (meaning they are continuously available throughout the co-simulation), HELICS messages are only readable once when collected from an endpoint. Once that collection is made the message only exists within the memory of the collecting message federate. If another message federate needs the information, a new message must be created and sent to the appropriate endpoint. Filters can be created to clone messages as well if that behavior is desired.

## Message Federate Endpoints

As previously discussed, message federates interact with the federation by defining an "endpoint" that acts as their address to send and receive messages. Message federates are typically sending and receiving measurements, control signals, commands, and other signal data with HELICS acting as a perfect communication system (infinite bandwidth, virtually no latency, guaranteed delivery).

In fact, as you'll see in [a later section](../../../fundamental_topics/filters.md), it is possible to create more realistic communication-system effects natively in HELICS (as well as use a full-blown communication simulator like [ns-3](https://www.nsnam.org) to do the same). This is relevant now, though, because it influences how the endpoints are created and, as a consequence, how the simulator handles messages. You could, for example, have a system with three federates communicating with each other: a remote voltage sensor, a voltage controller, and a voltage regulation actuator (we'll pretend for the case of this example that the last two are physically separated though they often aren't). In this case, you could imagine that the voltage sensor only sends messages to the voltage controller and the voltage controller only sends messages to the voltage regulation actuator. That is, those two paths between the three entities are distinct, have no interaction, and have unique properties (though they may not be modeled). Given this, referring to the figure below, the voltage sensor could have one endpoint ("Endpoint 1") to send the voltage signal, the voltage regulator could receive the measurement at one endpoint ("Endpoint 2") and send the control signal on another ("Endpoint 3"), and the voltage regulation actuator would receive the control signal on its own endpoint ("Endpoint 4").

![Voltage regulation message federates](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/voltage_reg_message_federate.png)

The federate code handling these messages can be relatively simple because the data coming in or going out of each endpoint is unique. The voltage controller always receives (and only receives) the voltage measurement at one endpoint and similarly only sends the control signal on another.

Consider a counter-example: automated meter-reading (AMI) using a wireless network that connects all meters in a distribution system to a data-aggregator in the substation (where, presumably, the data travels over a dedicated wired connection to a control room). All meters will have a single endpoint over which they will send their data but what about the receiver? The co-simulation could be designed with the data-aggregator having a unique endpoint for each meter but this implies come kind of dedicated communication channel for each meter; this is not the case with wireless communication. Instead, it is probably better to create a single endpoint representing the wireless connection the data aggregator has with the AMI network. In this case, messages from any of the meters in the system will be flowing through the same endpoint and to differentiate the messages from each other, the federate will have to be written to examine the metadata with the message to determine its original source.

![Signal topology for AMI message federates](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/ami_message_federate.png)

## Interactions Between Messages and Values

Though it is not possible to have a HELICS message show up at a value interface, the converse is possible; message_federates can subscribe to HELICS values. Every time a value federate publishes a new value to the federation, if a message federate has subscribed to that message HELICS will generate a new HELICS message and send it directly to the destination endpoint. These messages are queued and not overwritten (unlike in HELICS values) which means when a message federate is synchronized it may have multiple messages from the same source to manage.

This feature offers the convenience of allowing a message federate to receive messages from pure value federates that have no endpoints defined. This is particularly useful for simulators that do not support endpoints but are required to provide measurement signals controllers. Implemented in this way, though, it is not possible to later implement a full-blown communication simulator that these values-turned-messages can traverse. Such co-simulation architectures in HELICS require the existence of both a sending and receiving endpoint; this feature very explicitly by-passes the need for a sending endpoint.

## Example 1b - Distribution system EV charge controller

To demonstrate how a message federate interacts with the federation, let's take the previous example and add two things to it: add electric vehicle (EV) loads in the distribution system, and a centralized EV charge control manager. [Models files for this example can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/160409d079d5a95bc08d37e7eef76d4748f8e9a8/user_guide_examples/misc/gridlabd_example_1).

Keeping in mind that this a model for demonstration purposes (which is to say, don't take this too seriously), let's make the following assumptions and definitions to simplify the behavior of the EV charge controller:

- All EVs are very large (200kW; level 2 charging is rated up to 20kW so these are effective HVDC chargers)
- All EVs have infinite battery capacity
- All EVs will be at home all day, desiring to charge all day if they can.
- All EVs charge at the same power level.
- The EV charge controller has direct control over the charging of all EVs in the distribution system. It can tell them when to turn off and on at will.
- The EV charge controller has the responsibility to limit the total load of the distribution system to a specified level to prevent overloading on the substation transformer.
- The EV will turn off some EV charging when the total distribution load exceeds the transformer limit by a certain percentage and will turn some EVs back on when below the limit by a certain percentage.
- Nothing is fair about how the charge controller chooses which EVs to charge and which to disconnect.

The message topology (including the endpoints) and the not very interesting broker topology are shown below.

![Ex. 1b signal topology](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/Ex1b_Message_topology.png)

![Ex. 1b broker topology](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/Ex1b_Broker_topology.png)

Taking these assumptions and specifications, it is not too difficult to write a simple charge controller as a Python script. And just by opening the [JSON configuration file](https://github.com/GMLC-TDC/HELICS-Examples/blob/160409d079d5a95bc08d37e7eef76d4748f8e9a8/user_guide_examples/misc/gridlabd_example_1/1b_cosim_runner.json) we can learn important details about how the controller works.

```json
{
  "broker": true,
  "federates": [
    {
      "directory": ".",
      "exec": "python 1abc_Transmission_simulator.py -c 1b",
      "host": "localhost",
      "name": "1b_Transmission"
    },
    {
      "directory": ".",
      "exec": "python 1bc_EV_Controller.py -c 1b",
      "host": "localhost",
      "name": "1b_Controller"
    },
    {
      "directory": ".",
      "exec": "gridlabd.sh 1b_IEEE_123_feeder.glm",
      "host": "localhost",
      "name": "1b_GridLABD"
    }
  ],
  "name": "1b-T-D-Cosimulation-HELICSRunner"
}
```

The first thing to note is the the EV controller has been written as a combination federate, having both endpoints for receiving/sending messages and subscriptions to HELICS values. The HELICS values that the controller has subscribed to give the controller access to both the total load of the feeder (`totalLoad`, presumably) as well as the charging power for each of the individual EVs being controlled (six in total).

Looking at the [GridLAB-D JSON configuration file](https://github.com/GMLC-TDC/HELICS/tree/319de2b125fe5e36818f0434ac3d0a82ccc46534/examples/user_guide_examples/Example_1b/Distribution/IEEE_123_feeder_0.json) confirms this:

```json
{
  "coreInit": "--federates=1",
  "coreName": "Distribution Federate",
  "coreType": "zmq",
  "name": "DistributionSim",
  "offset": 0.0,
  "period": 60,
  "timeDelta": 1.0,
  "logfile": "output.log",
  "log_level": "warning",
  "publications": [
    {
      "global": true,
      "key": "IEEE_123_feeder_0/totalLoad",
      "type": "complex",
      "unit": "VA",
      "info": {
        "object": "network_node",
        "property": "distribution_load"
      }
    }
  ],
  "subscriptions": [
    {
      "required": true,
      "key": "TransmissionSim/transmission_voltage",
      "type": "complex",
      "unit": "V",
      "info": {
        "object": "network_node",
        "property": "positive_sequence_voltage"
      }
    }
  ],
  "endpoints": [
    {
      "global": true,
      "key": "IEEE_123_feeder_0/EV6",
      "destination": "EV_Controller/EV6",
      "info": {
        "publication_info": {
          "object": "EV6",
          "property": "constant_power_A"
        },
        "subscription_info": {
          "object": "EV6",
          "property": "constant_power_A"
        }
      }
    }
  ]
}
```

GridLAB-D is publishing out the total load on the feeder as well as the individual EV charging loads. It also has endpoints set up for each of the EV chargers to receive messages from the controller. Based on the strings in the `info` field it appears that the received messages are used to define the EV charge power.

Running [the example](https://github.com/GMLC-TDC/HELICS-Examples/blob/160409d079d5a95bc08d37e7eef76d4748f8e9a8/user_guide_examples/misc/gridlabd_example_1/1b_cosim_runner.json) and looking at the results, as the total load on the feeder exceeded the pre-defined maximum loading of the feeder (red line in the graph), the EV controller disconnected an additional EV load. Conversely, as the load dipped to the lower limit (green line), the controller reconnected the EV load. Looking at a graph of the EV charge power for each EV shows the timing of the EV charging for each load.

![Ex. 1b EV charge pattern](https://github.com/GMLC-TDC/helics_doc_resources/blob/db4e8a9edeb5602c6463ff147b8bc72e6119532e/user_guide/1b_EV_plot.png?raw=true)

Given the relatively dramatic changes in load, you might expect the voltage on the transmission system to be impacted.

![Ex. 1b transmission system voltage and load magnitude](https://github.com/GMLC-TDC/helics_doc_resources/blob/db4e8a9edeb5602c6463ff147b8bc72e6119532e/user_guide/1b_transmission_plot.png?raw=true)
