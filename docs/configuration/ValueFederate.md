# Value Federates

Value Federates provide the interfaces for registering publications and subscriptions and sending and receiving values through
those interfaces.  Publications and Subscriptions can be configured through API calls or through file configurations

## API calls

Publications and Subscriptions can be declared through ValueFederate methods or through Endpoint objects
these are defined in ValueFederate.hpp and Publications.hpp and Subscriptions.hpp.  For the MessageFederate api the register calls return
a publication_id_t or subscription_id_t value that must be used whenver the publication or subscription is referenced.  The object api contains those calls in a separate object


TODO:: add links to other generated documents

### file configuration

File based configuration looks primarily at an "publications" or "subscriptions" json array

```

```
