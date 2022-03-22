# Configuration in Complex Networks

## Default Operation using ZMQ core

The starting place for many HELICS-based co-simulation is running on a single computer using the default core, ZMQ. The ZMQ core provides a lot of advantages but its behavior on the localhost network is more complex and this complexity is often hidden when a user is only running the co-simulation on a single computer.

When starting up a HELICS-based co-simulation using the ZMQ core, HELICS opens two ports: one for high-priority traffic (default to port number 23405) and the other for low priority traffic (defaults to 23406). As federates join the co-simulation they connect to the broker on the high-priority port and the broker assigns the federate a dedicated port that all further communication between the broker and federate takes place.

Again, when all federates are running on a single computer, this prolific use of port numbers (at high federate counts) is generally not a problem. There may need to be permission granted on the local computer to open those ports in a firewall for localhost traffic but no traffic ever leaves the local machine.

## Speciality Cores for Complex Networks

When a co-simulation grows to the point where it starts spanning multiple compute nodes, the default ZMQ core may start running into networking problems. In situations where all the compute nodes are still in the same subnet or administered by the same organization, IT policies may easily accommodate the ZMQ core's need for ports. This may not always be the case, though, and to allow HELICS to operate in these environments, the HELICS developers have created two speciality cores to simplify the impact of HELICS in a networked environment.

### `zmq_ss` core

The zmq_ss core is a version of the ZMQ core with modified behavior to only use a single socket. (A socket is the combination of ip address and port.) The core has been designed to accommodate a large number of federates where there is a possibility of running out of available ports on a single compute node (ip address). By using a single socket, it has the side-affect of also simplifying the required networking and/or firewall configuration.

### `tcp_ss` core

The tcp_ss core is similar in nature to the zmq_ss core in that it uses a single socket but is based on the tcp core. This core removes the extra complexity of the zmq core and just uses the tcp protocol directly and has been designed as the go-to core when needing to work in complex networking environments. In addition to only using a single socket, the tcp core allows the broker to initiate connections with federates which can be important when trying to work in networking environments when firewalls prevent connections to be initiated in particular directions.

## `broker_address`, `broker_port`, `local_interface` and `local_port`

Regardless of which core you're using, there are a few specific networking options that allow for changes to default values to enable working in a more restrictive networking environment.

- **`broker_address`** and **`broker_port`** - for sub-brokers or federates, defines the IP address (`broker_address`) and port (`broker_port`) which should be used to connect to a parent broker
- **`local_interface`** and **`local_port`** - defines the IP address (`local_interface`) and port (`local_port`) where a broker, sub-broker, or federate will look for connections to the federation.

The `broker_port` is typically defined as a command line switch when instantiating the broker. `broker_address` and `broker_port` are also used as options when configuring federates to define the socket they should connect to. These options can be set as part of the `fed_init_string`, `core_init_string` or part of the JSON configuration for that federate. The `local_port` options can be similarly configured. Further details on these configuration methods can be found in the [Configuration Options Reference](../../references/configuration_options_reference).

It is also possible to include the port number when defining `broker_address` (effectively defining the broker's socket) in addition to the IP address such as in the format: `192.169.0.1:23400`. Doing so would then not require the `broker_port` option to be defined.

As of this writing there is a generic `port` option supported in HELICS that tries to be all things to all people. Experience has shown that though well-intentioned, the feature of it being generic has become a bug in that it causes confusion among users. You may still see it lurking in examples or documentation but it is recommended that its use be avoided and the more explicit `broker_port` and `local_port` be used instead.

## Broker Hierarchies and Multi-Computer Federations

Once a federation reaches a certain size, it is not unusual for it to end up deployed across multiple compute nodes and often this results in establishing a broker hierarchy to reduce traffic between compute nodes (and help the co-simulation to run faster). In complex networking environments, this will likely entail the use of the `tcp_ss` core and the specification of the broker and sub-broker sockets.

The good news is that we already have two other pages of documentation devoted to this and both include running examples that show how these features can be put to use. Here's the [documentation on broker hierarchies](./broker_hierarchies) and here's the one on [running across multiple compute nodes](./broker_multicomputer).
