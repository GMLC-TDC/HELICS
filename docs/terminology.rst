Terminology
===========

Basic
-----

1) Federate - An individual simulator that is computing something interesting and communicating with other simulators

2) Core - An object managing the interactions of one or more Federates

3) Broker - An object coordinating multiple cores or brokers
    - There can be several layers of brokers

4) Root Broker – the top broker on the hierarchy
    - Last chance router
    - Responsible for determining when to enter initialization mode for the federation

5) Federation – the set of all Federates executing together in a single co-simulation

Types of Federates
------------------

1) Value Federates

    - Direct Fixed Connections to other Federates
    - Physical values being sent back and forth
    - Associated Units

2) Message Federates

    - Packets of data
    - No fixed connections
    - For things such as Events, Communication packets, triggers

3) Combination Federate (Value+Message Federate)
