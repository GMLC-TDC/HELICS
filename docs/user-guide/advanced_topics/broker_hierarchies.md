# Broker Hierarchies

The simplest and most straight-forward way HELICS co-simulations are constructed is with a single broker. If all federates are running on a single compute node, a single broker is likely all you'll need. In situations where the co-simulation is running across multiple compute nodes, with a little bit of planning, the use of a hierarchy of brokers can help improve co-simulation performance.

## Why Multiple Brokers?

Brokers are primarily concerned with facilitating the message exchanges between federates. As the number of messages that must be passed increase, the load on the broker(s) increase as well. In cases where the co-simulation is deployed across multiple compute nodes, the total cost of sending messages over the network (primarily in terms of latency and the corresponding slowdown in federation performance) can become non-trivial. The worst case develops when the compute node hosting the broker is physically distant (and thus, experiences higher latency) from the compute nodes where the individual federates are running. If two federates running on the same node need to talk to each other but have a high-latency connection to the broker, their communication will be significantly hampered as they coordinate with that distant broker.

And thus the solution presents itself: a broker local to the compute node where the federates are located will have a very low latency connection and will generally experience a lower load of messages it needs to process. To receive this benefit, though, the co-simulation needs to be deployed to the compute nodes in such a way that the federates that talk to each other most frequently/heavily are located on the same node and a broker is also deployed to that node.

When federates join the federation they can be assigned to specific brokers and when the co-simulation begins, each local broker uses this information to route the messages it can. Any messages that it is not connected to, it sends up the broker hierarchy. The broker at the top of the hierarchy is the broker of last resort and it is guaranteed to be able to route all messages down the hierarchy to their intended destination.

So, using an example we've seen several times, imagine a scenario where a single transmission system covering the western US is being simulated with many, many individual neighborhood level power systems attached to that regional system and controllers manage a hypothetical fleet of electric vehicles (EVs) whose owners are in these neighborhoods. The EV controllers and the distribution system federates interact frequently and the distribution system federates and the transmission system federates also interact frequently.

The diagrams below show the message and broker topologies for this hypothetical examples.

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/broker_hierarchy_message_topology.png)

![](https://github.com/GMLC-TDC/helics_doc_resources/raw/main/user_guide/broker_hierarchy_broker_topology.png)

## Broker Hierarchy Implementation

To implement a broker hierarchy, modifications to the configuration files for the federates and command line options for the brokers need to be made.

For examples, the config JSON for the Distribution System A (where Broker A is at IP 127.0.0.1) would look something like this:

```json
{
  "name": "DistributionSystemA",
  "coreInit": "--broker_address=tcp://127.0.0.1"
}
```

The command line for launching Broker A also needs to be adjusted. For this examples, let's assume there is a total of 200 federates are expected by Broker A. Note that `broker_address` is used to define the address of the broker next up in the hierarchy; in this, case Broker C.

```shell-session
$ helics_broker -f200 --broker_address=tcp://127.0.0.127
```

The JSON config file for the Transmission and Generation System federate needs to indicate where it's broker (Broker C) is at (IP 127.0.0.127):

```json
{
  "name": "TransmissionGenerationSystem",
  "coreInit": "--broker_address=tcp://127.0.0.127"
}
```

Lastly, when broker C, the root broker, is instantiated, it may optionally specify the number of sub-brokers that are expected to connect to it (as well as the number of federates). Since it is the root broker, there is no parent broker address to specify.

```shell-session
$ helics_broker -f1 --sub_brokers=2
```

## Hierarchies with Complex Networks

In more complex networking environments ([see dedicated documentation](./networking)), it may be necessary to include an additional specification of the interface the broker, sub-broker, or federate would like to talk to the rest of the federation on. In these cases, typically only a port specification is added to the configuration. Adding the port can be done in one of two ways:

1. Appending a `:<port number>` after the IP address (_e.g._ `tcp://127.0.0.1:23405`)
2. Using `broker_port=<broker_port>` and/or `local_port=<local_port>`

`broker_port` is used to specify the port on which a broker or sub-broker should talk to its broker. `local_port` is used to define which port a broker, sub-broker, or federate will be listening on for its communication with the rest of the federation. For example, a federate may define its `local_port` as 23500; as HELICS is setting-up the federation anything that needs to talk to the federate in question will know to use port 23500. Again, this can be particularly important if firewalls and networking policies are making it difficult or impossible to connect in using the defaults.

Lastly, there is a full IP and/or socket specification that is analogous to `local_port`: `local_interface`. This is analogous to `broker_address` in that it allows the specification of an IP and/or a socket and is the complement to `local_port` in that it specifies how the federation could talk to the federate, broker, or sub-broker that is specifying it.

Using the above example, if the networking environment were more complex, the Distribution System A configuration could have been extended as follows to force the rest of the federation to talk to it on port 23500 as well as indicate the port its broker wants to use for communication (25000):

```json
{
  "name": "DistributionSystemA",
  "coreInit": "--broker_address=tcp://127.0.0.1:25000",
  "local_port": 23500
}
```

Similarly, Broker A could have been instantiated to force communication on certain ports:

```shell-session
$ helics_broker -f200 --broker_address=tcp://127.0.0.127:24000 --local_port=25000
```

Adding the port number to the broker address indicates Broker C (the broker for Broker A) should be contacted on port 24000 and that the rest of the federation (including Broker C) should contact Broker A on port 25000. Similarly, the transmission federate would need similar additional specification to contact Broker C. Note that the port number is just appended to the IP; this could have also been specified with a new line in the JSON file defining `broker_port`.

```json
{
  "name": "TransmissionGenerationSystem",
  "coreInit": "--broker_address=tcp://127.0.0.127:24000"
}
```

Broker B would need the same details so it could contact its parent broker and could additionally specify a unique port for its federates to contact it on:

```shell-session
$ helics_broker -f200 --broker_address=tcp://127.0.0.127:24000 --local_port=27000
```

Lastly, to complete the configuration implied by the above, Broker C would need to be called like this:

```shell-session
$ helics_broker -f1 --sub_brokers=2 --local_port=24000
```

## Example

A full co-simulation example showing how to implement a broker hierarchy [has been written up over here](../examples/advanced_examples/advanced_brokers_multibroker.md) (and the source code is in the [HELICS Examples repository](https://github.com/GMLC-TDC/HELICS-Examples/tree/main/user_guide_examples/advanced/advanced_brokers/hierarchies)).
