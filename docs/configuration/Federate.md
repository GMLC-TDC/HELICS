# Federate Configuration
General federate configuration consists of setting up the name, connectivity information, and timing information
This is generally done through a FederateInfo object and passing that into the construction function for a federate

## Federate Name
A federate name is a string that contains the name of the federate, it should be unique in the federation, if not included a random UID is 
automatically generated.  This name is prependend for any local interfaces

## Core information
Certain information is used by the federate to establish linkages to a core object this
### Core name
 the corename identifies a potentially preexisting core in the same process that can be used
or just names the created core. 

### Core type
see [Core Types](CoreTypes) for more details on the specific types of cores which are available and their purposes
but in general the core type defines the communication method used in the federation

### Coreinitstring
the core init string is a string used by any created core to establish connectivity with a broker
this includes port numbers, addresses, and the minimum number of federates

## Timing information
There are a number of parameters related to timing information in HELICS.  These deterimine what times the request time returns and how a federate handles interruptions and interacts with other federates
For a detailed description of the timing parameters see [Timing in Helics](Timing)

## Interface configuration
The interfaces (Publications, Subscriptions, Endpoints, and a Filters) are how a federate interacts with the larger federation
These can be set up through API calls or through Configuration Files
Json files can also contain information for the FederateInfo structure including timing and connectivity information

The specific different kinds of Federates define the patterns for different elements.  ValueFederates define the interfaces for publish and Subscribe mechanisms.
MessageFederates define interfaces for endpoints and the basic Federate contains API's for interacting with Filters
