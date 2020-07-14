# Broker Hierarchies

The simplest and most straight-forward way HELICS co-simulations are constructed is with a single broker. If all federates are running on a single compute node, a single broker is likely all you'll need. In situations where the co-simulation is running across multiple compute nodes, with a little bit of planning, the use of a hierarchy of brokers can help improve co-simulation performance.

## Why Multiple Brokers?

Brokers are primarily concerned with facilitating the message exchanges between federates. As the number of messages that must be passed increase, the load on the broker(s) increase as well. In cases where the co-simulation is deployed across multiple compute nodes, the total cost of sending messages over the network (primarily in terms of latency) can become non-trivial. The worst case develops when the compute node hosting the broker is physically distant (and thus, experiences higher latency) from the compute nodes where the individual federates are running. If two federates running on the same node that need to talk to each other but have a high-latency connection to the broker, their communication will be significantly hampered as they coordinate with that distant broker.

And thus the solution presents itself: a broker local to the compute node where the federates are located will have a very low latency connection and will generally experience a lower load of messages it needs to process. To receive this benefit, though, the co-simulation needs to be deployed to the compute nodes in such a way that the federates that talk to each other most frequently/heavily are located on the same node and a broker is also deployed to that node.

When federates join the federation they can be assigned to specific brokers and when the co-simulation begins, each local broker uses this information to route the messages it can. Any messages that it is not connected to, it sends up the broker hierarchy. The broker at the top of the hierarchy is the broker of last resort and it is guaranteed to be able to route all messages down the hierarchy to their intended destination.

So, using an example we've seen several times, imagine a scenario where a single transmission system covering the western US is being simulated with many, many individual neighborhood level power systems attached to that regional system and controllers manage a hypothetical fleet of electric vehicles (EVs) whose owners are in these neighborhoods. The EV controllers and the distribution system federates interact frequently and the distribution system federates and the transmission system federates also interact frequently.

The diagrams below show the message and broker topologies for this hypothetical examples.

![Message topology](../img/broker_hierarchy_message_topology.png)

![Broker topology](../img/broker_hierarchy_broker_topology.png)

## Broker Hierarchy Implementation

To implement a broker hierarchy, modifications to the configuration files for the federates and command line options for the brokers need to be made.

For examples, the config JSON for the Distribution System A (where Broker A is at IP 127.0.0.1) would look something like this:

```JSON
{
    "name" : "DistributionSystemA",
    "coreInit": "--broker_address=tcp://127.0.0.1"
    ...
}
```

The command line for launching Broker A also needs to be adjusted. For this examples, let's assume there is a total of 200 federates are expected by Broker A. Note that `broker_address` is used to define the address of the broker next up in the hierarchy; in this, case Broker C.

```sh
helics_broker -f200 --broker_address=tcp://127.0.0.127
```

Lastly, the JSON config file for the Transmission and Generation System federate needs to indicate where it's broker (Broker C) is at (IP 127.0.0.127):

```JSON
{
    "name" : "TransmissionGenerationSystem",
    "coreInit": "--broker_address=tcp://127.0.0.127"
    ...
}
```
