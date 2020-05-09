# Federate Configuration

General federate configuration consists of setting up the name, connectivity information, and timing information.
This is generally done through a `FederateInfo` object and passing that into the construction function for a federate.

## Federate Name

A federate name is a string that contains the name of the federate, it should be unique in the federation, if not included a random UID is automatically generated.
This name is prepended for any local interfaces.

## Core information

Certain information is used by the federate to establish linkages to a core object this.

### Core name

the corename identifies a potentially preexisting core in the same process that can be used
or just names the created core.

### Core type

See [Core Types](CoreTypes) for more details on the specific types of cores which are available and their purposes, but in general the core type defines the communication method used in the federation.

### Coreinitstring

The core init string is a string used by any created core to establish connectivity with a broker.
This includes port numbers, addresses, and the minimum number of federates. This is usually entered as a string containing command line arguments such as `"--timeout=2s --broker 192.168.2.1"`

## Timing information

There are a number of parameters related to timing information in HELICS.
These determine what times the request time returns and how a federate handles interruptions and interacts with other federates.
For a detailed description of the timing parameters see [Timing in Helics](./Timing.html).

## Interface configuration

The interfaces (Publications, Subscriptions, Endpoints, and a Filters) are how a federate interacts with the larger federation
These can be set up through API calls or through Configuration Files
Json files can also contain information for the FederateInfo structure including timing and connectivity information

The specific different kinds of Federates define the patterns for different elements. ValueFederates define the interfaces for publications and Input mechanisms.
MessageFederates define interfaces for endpoints and the basic Federate contains API's for interacting with Filters

Filters can be configured via files the following is an example of a JSON file. TOML configuration files are also supported. You can find [examples here](https://github.com/GMLC-TDC/HELICS-Examples/tree/bdbdf4/example_files)

```json

"filters":[
{
    "name":"filtername",  //filters can have names (optional)
    "sourcetargets":"ept1", // source target for the filter
    //"inputType":"genmessage",  //can trigger some warnings if there is mismatches for custom filters only used if operation is "custom"
    //"outputType":"genmessage",  //this could be useful if the filter actually translates the data and can be used to automatically order filters
    "operation":"delay", //currently valid operations are "delay","clone","cloning","timedelay","randomdelay","randomdrop","reroute","redirect","custom"
    "info":"this is an information string for use by the application",
    "properties":  //additional properties for filters are specified in a property array or object if there is just a single one
    {
        "name":"delay",  //A delay filter just has a single property
        "value":0.2    //times default to seconds though units can also be specified "200 ms" or similar
    }
},
{
    "name":"filtername2",  //filters can have names (optional)
    "sourcetargets":["filterFed/ept2"],  //this is a key field specifying the source targets can be an array
    //"dest":["dest targets"],  // field specifying destination targets
    "operation":"reroute", //currently valid operations are "delay","clone","cloning","timedelay","randomdelay","randomdrop","reroute","redirect","custom"
    "properties":  //additional properties for filters are specified in a property array or object if there is just a single one
    {
        "name":"newdestination",  //A reroute filter takes a new destination
        "value":"ept1"    //the value here is the endpoint that should be the new destination
    }
},
{
    "name":"filterClone",  //filters can have names (optional)
    "delivery":"ept2",  //cloning filters can have a delivery field
    "cloning":true,  //specify that this is cloning filter
    "properties":  //additional properties for filters are specified in a property array or object if there is just a single one
    [{
        "name":"destination",  //destination adds a cloning filter for all messages delivered to a particular
        "value":"ept1"    //the value here the endpoint that will have its messages cloned
    },
    {

        "name":"source",  //source adds a cloning filter for all messages send from a particular endpoint
        "value":"ept1"    //the value here the endpoint that will have its messages cloned
    }
    ]  //this pair of properties clone all messages to or from "ept1"  this could also be done in one property with "endpoint" but this seemed more instructive in this file
}
]

}

```

### Notes

The properties of a filter vary depending on the exact filter specified.

Valid modes are "source", "dest", "clone"

for source and dest filters valid operations include "delay", "reroute", "randdelay", "randomdrop", "clone", "custom"

for clone filters an operation of "clone" is assumed other specification result in errors on configuration.

"custom" filter operations usually require setting of a custom callback otherwise the filter won't do anything.
