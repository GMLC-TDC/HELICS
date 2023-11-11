# Example 1c: Filters and their Impacts on HELICS Messages

As was introduced in the [introductory section on federates](../../../fundamental_topics/federates.md), message federates (and combo federates) are used to send messages (control signals, measurements, anything traveling over some kind of communication network) via HELICS to other federates. Though they seem very similar, the way messages and values are handled by HELICS is very different and is motivated by the underlying reality they are being used to model.

1. **Messages are directed and unique, values are persistent.** - Because HELICS values are used to represent physical reality, they are available to any subscribing federate at any time. If the publishing federate only updates the value, say, once every minute, any subscribing federates that are granted a time during that minute window will all receive the same value regardless of when they requested it.

   HELICS messages, though, are much more like other kinds of real-world messages in that they are directed and unique. Messages are sent by a federate from one endpoint to another endpoint in the federation (presumably the receiving endpoint is owned by another federate but this doesn't have to be the case). Internal to HELICS, each message has a unique identifier and can be thought to travel between a generic communication system between the source and destination endpoints.

2. **Messages can be filtered, values cannot.** - By creating a generic communication system between two endpoints, HELICS has the ability to model simple communication system effects on messages traveling through said network. These effects are called "filters" and are associated with the HELICS core (which in turn manages the federate's endpoints) embedded with the federate in question. Typical filtering actions might be delaying the transmission of the message or randomly dropping a certain percentage of the received messages. Filters can also be defined to operate on messages being sent ("source filters") and/or messages being received ("destination filters").

   Because HELICS values do not pass through this generic network, they cannot be operated on by filters. Since HELICS values are used to represent physics of the system not the control and coordination of it, it is appropriate that filters not be available to modify them. It is entirely possible to use HELICS value federates to send control signals to other federates; co-simulations can and have been made to work in such ways. Doing so, though, cuts out the possibility of using filters and, as we'll see, the easy integration of communication system simulators.

The figure below is an example of a representation of the message topology of a generic co-simulation federation composed entirely of message federates. Source and destination filters have been implemented (indicated by the blue endpoints), each showing a different built-in HELICS filter function.

- As a reminder, a single endpoint can be used to both send and receive messages (as shown by Federate 4). Both a source filter and a destination filter can be set up on a single endpoint. In fact multiple source filters can be used on the same endpoint.
- The source filter on Federate 3 delays the messages to both Federate 2 and Federate 4 by the same 0.5 seconds. Without establishing a separate endpoint devoted to each federate, there is no way to produce different delays in the messages sent along these two paths.
- Because the filter on Federate 4 is a destination filter, the message it receives from Federate 3 is affected by the filter but the message it sends to Federate 2 is not affected.
- As constructed, the source filter on Federate 2 has no impact on this co-simulation as there are no messages sent from that endpoint.
- Individual filters can be targeted to act on multiple endpoints and act as both source and destination filters.

![Signal topology using built-in HELICS filters](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/messages_and_filters_example.png)

## Example 1c - EV charge controller with HELICS filters

To demonstrate the effects of filters, let's take the same model we were working with in the [previous example](../../../fundamental_topics/message_federates.md), and add a filter to the controller. Specifically, let's assume a very, very poor communication system and add a 3600 second delay to the control messages sent from the EV charge controller to each of the EVs. ([Model files for this example can be found here](https://github.com/GMLC-TDC/HELICS-Examples/tree/160409d079d5a95bc08d37e7eef76d4748f8e9a8/user_guide_examples/misc/gridlabd_example_1).)

![Ex. 1c signal topology](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/Ex1c_Message_topology.png)

Let's run [this co-simulation](https://github.com/GMLC-TDC/HELICS-Examples/blob/160409d079d5a95bc08d37e7eef76d4748f8e9a8/user_guide_examples/misc/gridlabd_example_1/1c_cosim_runner.json) and capture the same data as last time for direct comparison: total substation load and EV charging behavior, both as a function of time.

![Ex. 1c total feeder load and voltage](https://github.com/GMLC-TDC/helics_doc_resources/blob/db4e8a9edeb5602c6463ff147b8bc72e6119532e/user_guide/1c_transmission_plot.png?raw=true)

![Ex. 1c EV charge pattern](https://github.com/GMLC-TDC/helics_doc_resources/blob/db4e8a9edeb5602c6463ff147b8bc72e6119532e/user_guide/1c_EV_plot.png?raw=true)

Granted that the charge controller communication system is ridiculously poor, this example does show that communication system effects can have a significant impact on system operation. For more realistic example, the HELICS Use Case repository has [an example](https://github.com/GMLC-TDC/HELICS-Use-Cases/tree/main/PNNL-Wide-Area-Control) of frequency control using real-time PMU measurements that shows the impact of imperfect communication systems.
