# Value Federates

Value Federates provide the interfaces for registering publications and subscriptions and sending and receiving values through
those interfaces.  Publications and Subscriptions can be configured through API calls or through file configurations

## API calls

Publications and Subscriptions can be declared through ValueFederate methods or through Endpoint objects
these are defined in ValueFederate.hpp and Publications.hpp and Subscriptions.hpp.  For the MessageFederate api the register calls return
a publication_id_t or subscription_id_t value that must be used whenever the publication or subscription is referenced.  The object api contains those calls in a separate object


TODO:: add links to other generated documents

### file configuration

File based configuration looks primarily at an "publications" or "subscriptions" JSON array

```
//this should be a valid JSON file (except comments are not recognized in standard JSON)
{ //example JSON configuration file for a value federate all arguments are optional
    "name":"valueFed", // the name of the federate
    //possible flags
    "observer":false,  // indicator that the federate does not send anything
    "rollback": false, // indicator that the federate can use rollback -NOTE: not used at present
    "only_update_on_change":false, //indicator that the federate should only indicate updated values on change
    "only_transmit_on_change":false,  //indicator that the federate should only publish if the value changed
    "source_only":false,  //indicator that the federate is only a source and is not expected to receive anything
    "uninterruptible":false, //indicator that the federate should only return requested times
    "coreType":"test", //the type of the core "test","zmq","udp","icp","tcp","mpi"
    "coreName":"the name of the core",  //this matters most for icp and test cores, can be empty
    "coreInit":"1", // the initialization string for the core in the form of a command line arguments
    "maxIterations":10, //the maximum number of iterations for a time step
    "period":  1.0, //the period with which federate may return time
    "offset": 0.0, // the offset shift in the period
    "timeDelta":0.0, // the minimum time between subsequent return times
    "outputDelay":0, //the propagation delay for federates to send data
    "inputDelay":0, //the input delay for external data to propagate to federates

//Publications used in the federate
"publications":[
{
    "key":"pub1", // the name of the publication
    "type":"double", // the type assocaited with a publication (optional)
    "unit":"m",  // the units associated with a publication (optional)
    "global":true //set to true to make the key global (default is false in which case the publication is prepended with the federate name)
},
{
    "key":"pub2", // the name of the publication
    "type":"double" // the type assocaited with a publication (optional)
    //no global:true implies this will have the federate name prepended like valueFed/pub2
}
],
//subscriptions used in the federate
"subscriptions":[
{
    "key":"pub1", // the key of the publication
    "type":"double", // the type assocaited with a publication (optional)
    "required":true //set to true to make helics issue a warning if the publication is not found
},
{
    "key":"fedName/pub2", // the name of the publication to subscribe to
    "shortcut":"pubshortcut"  //a naming shortcut for the publication for later retrieval
}
]

}

```
###### Notes
Shortcuts just provide a shortcut name for later reference instead of having to use a potentially longer key, the shortcut is only relevant inside a single federate

