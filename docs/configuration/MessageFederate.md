# Message Federates

Message Federates provide the interfaces for registering endpoints and sending and receiving messages through
those endpoints.  Endpoints can be configured through API calls or through file configurations

## API calls

endpoints can be declared through MessageFederate methods or through Endpoint objects
these are defined in MessageFederate.hpp and Endpoints.hpp.  For the MessageFederate api the register calls return
and endpoint_id_t value that must be used whenver the endpoint is referenced.  The Endpoint object api contains those calls in a separate object


TODO:: add links to other generated documents

### file configuration

File based configuration looks primarily at an "endpoints" json array

```

```
