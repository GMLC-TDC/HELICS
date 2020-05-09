# Message Federates

Message Federates provide the interfaces for registering endpoints and sending and receiving messages through those endpoints.
Endpoints can be configured through API calls or through file configurations

## API calls

Endpoints can be declared through MessageFederate methods or through Endpoint objects.
These are defined in MessageFederate.hpp and Endpoints.hpp.
For the MessageFederate api the register calls return and `endpoint_id_t` value that must be used whenever the endpoint is referenced.
The Endpoint object api contains those calls in a separate object.

<!-- TODO:: add links to other generated documents -->

### file configuration

File based configuration looks primarily at an "endpoints" JSON array

```json
//this should be a valid JSON file (except comments are not recognized in standard JSON)
{
  //example JSON configuration file for a message federate all arguments are optional
  "name": "messageFed", // the name of the federate
  //possible flags
  "observer": false, // indicator that the federate does not send anything
  "rollback": false, // indicator that the federate can use rollback -NOTE: not used at present
  "only_update_on_change": false, //indicator that the federate should only indicate updated values on change
  "only_transmit_on_change": false, //indicator that the federate should only publish if the value changed
  "source_only": false, //indicator that the federate is only a source and is not expected to receive anything
  "uninterruptible": false, //indicator that the federate should only return requested times
  "coreType": "test", //the type of the core "test","zmq","udp","ipc","tcp","mpi"
  "coreName": "the name of the core", //this matters most for ipc and test cores, can be empty
  "coreInit": "1", // the initialization string for the core in the form of a command line arguments
  "maxIterations": 10, //the maximum number of iterations for a time step
  "period": 1.0, //the period with which federate may return time
  "offset": 0.0, // the offset shift in the period
  "timeDelta": 0.0, // the minimum time between subsequent return times
  "outputDelay": 0, //the propagation delay for federates to send data
  "inputDelay": 0, //the input delay for external data to propagate to federates

  //endpoints used in the federate
  "endpoints": [
    {
      "name": "ept1", // the name of the publication
      "type": "genmessage", // the type associated with a endpoint endpoint types have limited usefulness at present (optional)
      "global": true //set to true to make the key global (default is false in which case the publication is prepended with the federate name)
    },
    {
      "name": "ept2", // the name of the publication
      "type": "message2", // the type associated with a endpoint (optional)
      //the fact that there is no global value creates a local endpoint with global name messageFed/ept2
      "knownDestinations": "ept1", //this value can be an array of strings or just a single one it names key paths
      //knownDestinations can be used to optimize the communication pathways inside of HELICS
      "subscriptions": "fed2/sub1" //subscribe an endpoint to a particular publication  this means that an endpoint will get a message whenever anything is published to that particular key
      //the message will be raw data so it would have to be translated to be useful. this can also be a JSON array to subscribe to multiple publications
    }
  ]
}
```

See the comments in the file for more information.
Endpoints can subscribe to publications in which case a message is delivered for every value published.
