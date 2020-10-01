
   Using our example of an integrated transmission and distribution powerflow, its easy to think of each distribution circuit acting as a load on the transmission system and the transmission system, since most (sometimes all) of the generation is directly attached to it, acting as the supplier of energy for the distribution systems. Our co-simulation design, then, defines that the transmission system will provide voltages to the distribution system (as a result of solving its powerflow) and the distribution system will provide load values to the transmission system after using the provided voltages as their substation voltage and solving their powerflow.

   If you're familiar with transmission and distribution system simulation tools, you might have already realized that there is a bit of a mismatch in how those tools operate that creates a small problem in our design: typically transmission system solvers assuming a balanced (positive-sequence only) network while distribution systems are often more comprehensive and model all three phases and support imbalanced operation. This implies that the voltages being supplied to the distribution system will always be balanced and only the positive-sequence component of the distribution system load can be used by the transmission system. When defining the values that will be sent as messages between federates, it is important that these modeling differences be taken into account.

   Given the fact that only two federates are being used (the minimum number to be a co-simulation, though HELICS itself works just fine with a single federate), only a single broker is required.

   ![Transmission and distribution co-simulation signal topology](../../img/ditl_message_topology.png)

   ![Transmission distribution co-simulation broker topology](../../img/ditl_broker_topology.png)



   Below is an example of how a very generic configuration for the transmission federate could look followed by one for the distribution federate.

```json
    {
        "name":"transmission_federate",
        "coreType":"ZMQ"
        "publications":[
        {
            "key":"transmission_voltages",
            "type":"double",
            "unit":"V",
        }],
        "subscriptions":[
        {
            "key":"distribution_federate/distribution_loads",
            "type":"double",
            "required":true
        }],
    }
```

```json
    {
        "name":"distribution_federate",
        "coreType":"ZMQ"
        "publications":[
        {
            "key":"distribution_loads",
            "type":"double",
            "unit":"W",
        }],
        "subscriptions":[
        {
            "key":"transmission_federate/transmission_voltages",
            "type":"double",
            "required":true
        }],
    }
```


   In the case of running our integrated transmission and distribution powerflow, both federates only need to update when the other provides a new value. Effectively, the time request will determine the temporal resolution of the simulation. If loadshapes or historical data is being used by the distribution system to determine individual load co-efficients, the resolution of that data would define a lower bound for the temporal resolution. For this example, let's assume each federate only needs to update every five minutes.


   In our example, it may be important that when the co-simulation proper begins (t = 0) that the loads and resulting voltages that are solved for across both the transmission and distribution systems are in agreement. Or, depending on the specific purpose of the co-simulation, it may be acceptable for the first few powerflows to be less precise as the federates step through time towards a more consistent state across the federation.


   In our powerflow example, every granted time both the transmission and distribution system use the previously published values from the other federate as a new input value (boundary condition) and runs a powerflow, re-solving their system. As you may have noticed, this can easily produce an inconsistent state as each federate is using data from the last time period (say, t = 10 minutes) to solve the state for this period (t = 15 minutes). Because new values are received by the federate only once a time has been granted and the granted time is typically forward in time, the data being received will generally be out of step like this. HELICS does support a re-iteration mode that allows the granted time to be the same as the current time; this is discussed in more detail in section [timing](./timing.md).