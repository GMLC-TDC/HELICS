# Filters
Filters are interfaces which can modify messages including routes,
destinations, times, existence, and payloads.  This is useful for
inserting communication modules into a data stream as an optional component
they can also be used to clone messages and randomly drop them.

### Filter Creation
Filters are registered with the core or through the application API.
There are also Filter object that hide some of the api calls in a slightly nicer interface
Generally a filter will define a target endpoint as either a source filter or destination filter
Source filters can be chained, as in there can be more than one of them.  At present there can only be a
single non-cloning destination filter attached to an endpoint. 

Non-cloning filters can modify the message in some way,  cloning filters just copy the message and may send it to multiple destinations.

on creation filters have a target endpoint and an optional name.  Custom filters may have input and output types associated with them.  
This is used for chaining and automatic ordering of filters.  Filters do not have to be defiened on the same core as the endpoint, and in fact can be 
anywhere in the federation, any messages will be automatically routed appropriately.

### predefined filters
Several predefined filters are available, these are parameterized so they can be tailored to suite the simulation needs

##### reroute
This filter reroutes a message to a new destination. it also has an optional filtering mechanism that will only reroute if 
some patterns are matching

##### delay
This filter will delay a message by a certain amount

##### randomdelay
This filter will randomly delay a message according to specified random distribution
available options include distribution selection, and 2 paramaters for the distribution
some distributions only take one parameter in which case the second is ignored.

##### randomdrop
This filter will randomly drop a message, the drop probability is specified, and is modeled as a uniform distribution

##### clone
this message will copy a message and send it to the original destination plus a new one.  

##### custom filters
Custom filters are allowed as well, these require a callback operator that can be called from any thread
and modify the message in some way.   